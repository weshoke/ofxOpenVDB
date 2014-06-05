#pragma once

#include <set>
#include <openvdb/tools/RayIntersector.h>
#include <openvdb/math/Ray.h>
#include <openvdb/math/Stencils.h>


using std::set;
using openvdb::Coord;
using openvdb::Vec3d;
using openvdb::Vec4d;
using openvdb::math::Ray;
using openvdb::math::BaseStencil;
using openvdb::math::BoxStencil;


// Copied from <openvdb/tools/RayIntersector.h>
template<typename GridT,
         typename SearchImplT = openvdb::tools::LinearSearchImpl<GridT>,
         int NodeLevel = GridT::TreeType::RootNodeType::ChildNodeType::LEVEL,
         typename RayT = Ray<openvdb::Real> >
class IndexMeshRayIntersector
{
public:
    typedef GridT                         GridType;
    typedef RayT                          RayType;
    typedef typename RayT::Vec3T          Vec3Type;
    typedef typename GridT::ValueType     ValueT;
    typedef typename GridT::TreeType      TreeT;

    BOOST_STATIC_ASSERT(NodeLevel >= -1 && NodeLevel < int(TreeT::DEPTH)-1);

    /// @brief Constructor
    /// @param grid level set grid to intersect rays against
    IndexMeshRayIntersector(const GridT& grid): mTester(grid)
    {
        if (!grid.hasUniformVoxels() ) {
            OPENVDB_THROW(openvdb::RuntimeError, "IndexMeshRayIntersector only supports uniform voxels!");
        }
    }
    
    /// @brief Return @c true if the index-space ray intersects the level set
    /// @param iRay ray represented in index space
    bool intersectsIS(const RayType& iRay) const
    {
        if (!mTester.setIndexRay(iRay)) return false;//missed bbox
        return openvdb::math::LevelSetHDDA<TreeT, NodeLevel>::test(mTester);
    }
    
    /// @brief Return @c true if the index-space ray intersects the level set.
    /// @param iRay ray represented in index space.
    /// @param xyz  if an intersection was found it is assigned the
    ///             intersection point in index space, otherwise it is unchanged.
    bool intersectsIS(const RayType& iRay, Vec3Type& xyz) const
    {
        if (!mTester.setIndexRay(iRay)) return false;//missed bbox
        if (!openvdb::math::LevelSetHDDA<TreeT, NodeLevel>::test(mTester)) return false;
        mTester.getCell(xyz);
        return true;
    }
	
	void ignoreCoord(const Coord& ijk) { mTester.ignoreCoord(ijk); }
    
private:

    mutable SearchImplT mTester;
};


template<>
bool BoxStencil<openvdb::Int64Grid>::intersects(const ValueType &isoValue) const {
	std::cout << "test: " << mCenter.x() << " " << mCenter.y() << " " << mCenter.z() << "\n";
	return mGrid->getAccessor().isValueOn(mCenter);
}


template<typename GridT, int Iterations = 0, typename RealT = double>
class MarkedLinearSearchImpl;


template<typename GridT, int Iterations, typename RealT>
class MarkedLinearSearchImpl
{
public:
    typedef openvdb::math::Ray<RealT>			RayT;
    typedef typename GridT::ValueType			ValueT;
    typedef typename GridT::ConstAccessor		AccessorT;
    typedef openvdb::math::BoxStencil<GridT>	StencilT;
    typedef typename StencilT::Vec3Type			Vec3T;

    /// @brief Constructor from a grid.
    /// @throw RunTimeError if the grid is empty.
    MarkedLinearSearchImpl(const GridT& grid)
        : mStencil(grid)
      {
          if ( grid.empty() ) {
              OPENVDB_THROW(openvdb::RuntimeError, "MarkedLinearSearchImpl does not supports empty grids");
          }
          
		  grid.tree().root().evalActiveBoundingBox(mBBox, /*visit individual voxels*/false);
      }

    /// @brief Return @c false the ray misses the bbox of the grid.
    /// @param iRay Ray represented in index space.
    /// @warning Call this method before the ray traversal starts.
    inline bool setIndexRay(const RayT& iRay)
    {
        mRay = iRay;
        return mRay.clip(mBBox);//did it hit the bbox
    }

    /// @brief Return @c false the ray misses the bbox of the grid.
    /// @param wRay Ray represented in world space.
    /// @warning Call this method before the ray traversal starts.
    inline bool setWorldRay(const RayT& wRay)
    {
        mRay = wRay.worldToIndex(mStencil.grid());
        return mRay.clip(mBBox);//did it hit the bbox
    }

    /// @brief Get the intersection point in index space.
    /// @param xyz The position in index space of the intersection.
    inline void getIndexPos(Vec3d& xyz) const { xyz = mRay(mTime); }

    /// @brief Get the intersection point in world space.
    /// @param xyz The position in world space of the intersection.
    inline void getWorldPos(Vec3d& xyz) const { xyz = mStencil.grid().indexToWorld(mRay(mTime)); }

	/// @brief Return the time of intersection along the index ray.
    inline RealT getIndexTime() const { return mTime; }

    /// @brief Return the time of intersection along the world ray.
    inline RealT getWorldTime() const
    {
        return mTime*mStencil.grid().transform().baseMap()->applyJacobian(mRay.dir()).length();
    }
	
	inline void getCell(Vec3d& c) const
	{
		c[0] = mCell[0];
		c[1] = mCell[1];
		c[2] = mCell[2];
	}
	
	void ignoreCoord(const Coord& ijk) { ignore.insert(ijk); }

private:

    /// @brief Initiate the local voxel intersection test.
    /// @warning Make sure to call this method before the local voxel intersection test.
    inline void init(RealT t0)
    {
        mT[0] = t0;
    }

    inline void setRange(RealT t0, RealT t1) { mRay.setTimes(t0, t1); }

    /// @brief Return a const reference to the ray.
    inline const RayT& ray() const { return mRay; }

    /// @brief Return true if a node of the the specified type exists at ijk.
    template <typename NodeT>
    inline bool hasNode(const Coord& ijk)
    {
        return mStencil.accessor().template probeConstNode<NodeT>(ijk) != NULL;
    }

    /// @brief Return @c true if an intersection is detected.
    /// @param ijk Grid coordinate of the node origin or voxel being tested.
    /// @param time Time along the index ray being tested.
    /// @warning Only if and intersection is detected is it safe to
    /// call getIndexPos, getWorldPos and getWorldPosAndNml!
    inline bool operator()(const Coord& ijk, RealT time)
    {
		std::cout << "MarkedLinearSearchImpl\n";
		if(ignore.find(ijk) != ignore.end()) {
			std::cout << "IGNORE: " << ijk << "\n";
			return false;
		}
	
		std::cout << ijk << " " << mRay(time) << " " << (mStencil.accessor().isValueOn(ijk) ? "ON" : "OFF") << "\n";
		if(mStencil.accessor().isValueOn(ijk)) {
			mT[0] = time;
			mT[1] = time;
			mTime = time;
			mCell = ijk;
			return true;
		}
		return false;
	}

    template<typename, int> friend struct openvdb::math::LevelSetHDDA;

    RayT            mRay;
    StencilT        mStencil;
    RealT           mTime;//time of intersection
    RealT           mT[2];
    openvdb::math::CoordBBox mBBox;
	Coord			mCell;
	set<Coord>		ignore;
};


/*
template<typename GridT, int Iterations, typename RealT>
class MarkedLinearSearchImpl : public openvdb::tools::LinearSearchImpl<GridT, Iterations, RealT> {
public:
	MarkedLinearSearchImpl(const GridT& grid)
	:	openvdb::tools::LinearSearchImpl<GridT, Iterations, RealT>(grid)
	{}
	
	MarkedLinearSearchImpl(const set<Coord>& ignore, const GridT& grid)
	:	ignore(ignore),
		openvdb::tools::LinearSearchImpl<GridT, Iterations, RealT>(grid)
	{}
	
	bool operator()(const Coord& ijk, double time)
	{
		//std::cout << "MarkedLinearSearchImpl\n";
		if(ignore.find(ijk) != ignore.end()) {
			//std::cout << "IGNORE: " << ijk << "\n";
			return false;
		}
		else return openvdb::tools::LinearSearchImpl<GridT, Iterations, RealT>::operator()(ijk, time);
	}
	
	void ignoreCoord(const Coord& ijk) { ignore.insert(ijk); }
	
protected:
	set<Coord> ignore;
};
*/
