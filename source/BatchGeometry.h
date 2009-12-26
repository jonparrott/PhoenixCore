/*

Copyright (c) 2009, Jonathan Wayne Parrott

Please see the license.txt file included with this source
distribution for more information.

*/


#ifndef __PHBATCHGEOMETRY_H__
#define __PHBATCHGEOMETRY_H__

#include <vector>
#include <GL/glfw.h>
#include "Vertex.h"
#include "Texture.h"
#include "Rectangle.h"
#include "TrackingInvariant.h"
#include "BatchRenderer.h"
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

namespace phoenix
{

//! Batch Geometry Class.
/*
	This class is used by the optimizing batch renderer to display geometry.
	Users are able to overload this class and highly customize it. This class
	is garbage collected and managed in very similar manner to Resource, but
	it should be noted that they are not siblings. Geometry is organized in
	the BatchRenderer by depth, group, texture id, and primitive type. Any changes
	to any of these properties must be followed by an update() call.
*/
class BatchGeometry
	: public boost::enable_shared_from_this<BatchGeometry>, public virtual Droppable
{

public:


	
	//! Create
	/*!
		Factory for creating new geometry. The geometry is automatically added to the given BatchRenderer.
		\param _r The batch renderer that will display this geometry.
		\param _p The primitive type.
		\param _t The texture.
		\param _g The group id.
		\param _d The depth.
	*/
	static boost::shared_ptr< BatchGeometry > create( BatchRenderer& _r, unsigned int _p = GL_QUADS, boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>(), signed int _g = 0, float _d = 0.0f )
	{
		boost::shared_ptr< BatchGeometry > newgeom( new BatchGeometry( _r, _p, _t, _g, _d ) );
		_r.addGeometry( newgeom->grab<BatchGeometry>() );
		return newgeom->grab<BatchGeometry>();
	}

	//! Create from Rectangle
	/*!
		Factory for creating new geometry. This helper factory creates geometry from a rectangle.
		The type automatically defaults to GL_QUADS. The texture coordinates are automatically set
		to (0,0), (0,1), (1,1), (1,0).
		The geometry is automatically added to the given BatchRenderer.
		\param _r The batch renderer that will display this geometry.
		\param _rect The rectangle.
		\param _t The texture.
		\param _g The group id.
		\param _d The depth.
	*/
	static boost::shared_ptr< BatchGeometry > create( BatchRenderer& _r, const Rectangle& _rect, boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>(), signed int _g = 0, float _d = 0.0f )
	{

		boost::shared_ptr< BatchGeometry > newgeom( new BatchGeometry( _r, GL_QUADS, _t, _g, _d ) );
		newgeom->fromRectangle( _rect );
		_r.addGeometry( newgeom->grab<BatchGeometry>() );
		return newgeom->grab<BatchGeometry>();
	}

	//! Create from Polygon
	/*!
		Factory for creating new geometry. This helper factory creates geometry from a polygon.
		The type automatically defaults to GL_TRIANGLES. No texture coordinates are generated.
		The geometry is automatically added to the given BatchRenderer.
		\param _r The batch renderer that will display this geometry.
		\param _poly The polygon.
		\param _t The texture.
		\param _g The group id.
		\param _d The depth.
	*/
	static boost::shared_ptr< BatchGeometry > create( BatchRenderer& _r, const Polygon& _poly, boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>(), signed int _g = 0, float _d = 0.0f )
	{

		boost::shared_ptr< BatchGeometry > newgeom( new BatchGeometry( _r, GL_TRIANGLES, _t, _g, _d ) );
		newgeom->fromPolygon( _poly );
		_r.addGeometry( newgeom->grab<BatchGeometry>() );
		return newgeom->grab<BatchGeometry>();
	}

	/*!
		Does nothing.
		\sa drop()
	*/
	virtual ~BatchGeometry()
	{
	}

	//! Get vertex
	/*!
		\note This function works like a ring buffer. this is to reduce the complexity of certain geometric algorithms. 
	*/
	inline const Vertex& getVertex(const signed int& a) const
    {
        return vertices[ a % vertices.size() ];
    }

	//! Set vertex.
    /*!
		\note This function acts like a ring buffer, this is to reduce the complexity of implementing some geometric algorithms.
    */
    inline void setVertex(const signed int& a, const Vertex& v)
    {
        vertices[ a % vertices.size() ] = v;
    }


	//! Add vertex.
    inline void addVertex(const Vertex& a)
    {
        vertices.push_back(a);
    }

	//! Remove vertex.
	/*!
		\note This function works like a ring buffer. this is to reduce the complexity of certain geometric algorithms. 
	*/
	inline void removeVertex( const signed int& a )
	{
		vertices.erase( vertices.begin() + (a % vertices.size()) );
	}

	//! The current number of vertices in the geometry.
    inline const unsigned int getVertexCount() const
    {
        return vertices.size();
    }

	//! Array operator for vertices.
	inline Vertex& operator[] ( signed int _i ) { return vertices[ _i % vertices.size() ]; }

	//! Returns the key invariant for the Primitive Type (used by BatchRender).
	inline TrackingInvariant< unsigned int >& getPrimitiveTypeInvariant() { return primitivetype; }

	//! Returns the key invariant for the Textured ID (used by BatchRender).
	inline TrackingInvariant< unsigned int >& getTextureIdInvariant() { return textureid; }

	//! Returns the key invariant for the Group ID (used by BatchRender).
	inline TrackingInvariant< signed int >& getGroupInvariant() { return groupid; }

	//! Returns the key invariant for the Depth (used by BatchRender).
	inline TrackingInvariant< float >& getDepthInvariant() { return depth; }

	//! Get the texture associated with this geometry.
	inline boost::shared_ptr< Texture > getTexture() { return texture; }

	//! Get the OpenGL Primitive Type associated with this geometry.
	inline const unsigned int& getPrimitiveType() const { return primitivetype; }

	//! Get the OpenGL Texture ID associated with this geometry.
	inline const unsigned int& getTextureId() const { return textureid; }

	//! Get the Group ID associated with this geometry.
	inline const signed int& getGroup() const { return groupid; }

	//! Get the Depth associated with this geometry.
	inline float getDepth() const { return depth; }

	//! Check if this geometry is enabled.
	inline bool getEnabled() const { return enabled; }

	//! Check if this geometry is immediate.
	inline bool getImmediate() const { return immediate; }

	//! Set OpenGL Primitive type.
	/*
		Sets OpenGL primitive type of this piece of geometry. It defaults to
		GL_QUADS, but may be GL_TRIANGLES, GL_LINES, GL_POINTS, etc.
		\see getPrimitiveType(), update()
	*/
	inline void setPrimitiveType( const unsigned int& _v ) { primitivetype = _v; }

	//! Set Texture.
	/*!
		Sets the texture for this geometry. If it is an invalid texture or empty
		pointer, the Id is set to 0 to disable texturing for this geometry. 
		\see getTexture(), getTextureId(), update()
	*/
	inline void setTexture( boost::shared_ptr< Texture > _t ) 
	{ 
		texture = _t; 
		textureid = _t ? _t->getTextureId() : 0 ;
	}

	//! Set Group Id.
	/*
		Sets the Group identifier of this geometry. Geometries of the same group usually have the same properties and
		share render states that are set and unset via the groupEnd() and groupBegin() functions. All geometry of
		the same group id should have the same groupEnd() and groupBegin() functions.
		\see getGroup(), update(), groupEnd(), groupBegin()
	*/
	inline void setGroup( const signed int& _v ) { groupid = _v; }

	//! Set Depth
	/*!
		Sets the rendering depth of this geometry.
		\see getDepth(), update()
	*/
	inline void setDepth( float _v ) { depth = _v; }

	//! Enable or Disable.
	/*!
		Disabled geometry is skipped over during rendering.
		\see getEnabled()
	*/
	inline void setEnabled( bool _e ) { enabled = _e; }

	//! Immediate rendering.
	/*!
		Immediate geometry is drawn once then dropped, so this is useful for non-persistent geometry. It's highly
		suggested that you only use this when needed, as persistent geometry is much faster.
		\see getImmediate(), drop()
	*/
	inline void setImmediate( bool _i ) { immediate = _i; }

	//! Set group begin function.
	/*!
		The given function is called by the renderer on the first geometry of any given group before that group is drawn. 
		It should establish common renderstate of all members of the group (such as a new blending mode). All 
		geometry of the same type should have the same group functions.
		\see groupBegin(), setGroupEndFunction()
	*/
	inline void setGroupBeginFunction( boost::function< void() > _f ) { groupbegin = _f; }

	//! Set group end function.
	/*!
		The given function is called by the renderer on the first geometry of any given group after that group is drawn. 
		It should restore anything set by the group begin function (such as restoring a blending mode). All 
		geometry of the same type should have the same group functions.
		\see groupEnd(), setGroupBeginFunction()
	*/
	inline void setGroupEndFunction( boost::function< void() > _f ) { groupend = _f; }

	//! Update
	/*!
		This function will check all invariants and move the geometry's location in the renderer's graph
		if any of them have been broken, and then resets all the invariants. This function should be called
		after the depth, group, texture, or primitive type of this geometry is changed.
	*/
	void update()
	{
		if( ! (primitivetype.check() && textureid.check() && groupid.check() && depth.check()) )
		{
#ifdef DEBUG_BATCHGEOMETRY
			std::cout<<"\n - Invariant "<<primitivetype.check()<<textureid.check()<<groupid.check()<<depth.check()<<" failed on "<<this<<std::endl;
#endif
			renderer.moveGeometry( grab<BatchGeometry>() );
			primitivetype.reset();
			textureid.reset();
			groupid.reset();
			depth.reset();
		}
	}

	//! Grab
	/*!
		Grabs a pointer to this geometry. This is very akin to Resource::grab().
	*/
	template <typename T>
    inline boost::shared_ptr<T> grab()
    {
        try
        {
            return boost::static_pointer_cast<T,BatchGeometry>( shared_from_this() );
        }
        catch ( ... )
        {
            return boost::shared_ptr<T>();
        }
    }

	//! Drop
	/*!
		Drops this geometry from the renderer, and adds it to the garbage collection
		list. The geometry should be considered deleted. Derived classes should
		call this manually. This is very akin to Resource::drop().
		\sa dropped(), grab(), Droppable::drop(), Resource::drop()
	*/
	inline virtual void drop()
	{
		if( ! dropped() )
		{
			Droppable::drop();
			renderer.removeGeometry( grab<BatchGeometry>() );
		}
	}

	//! Batch
	/*!
		This function is called by BatchRenderer when it draws the current render
		graph. The geometry should append all of it's vertices to the renderer's
		list.
		\param list The current vertex list from the current BatchRender.
		\returns The number of elements added (this is currently optional).
	*/
	virtual unsigned int batch( std::vector<Vertex>& list )
	{
		if( immediate )
		{
			drop();
		}

		if( enabled )
		{
			if( ! vertices.empty() )
			{
				BOOST_FOREACH( Vertex& v, vertices )
					list.push_back( v );
			}
			return 1;
		}
		return 0;
	}

	//! Call Group begin function
	/*!
		This is called by BatchRender for the first member of every group before drawing. It calls the function specified by
		setGroupBeginFunction(). 
		@see groupEnd()
	*/
	virtual void groupBegin()
	{
		if( groupbegin )
			groupbegin();
	}

	//! Call Group begin function
	/*!
		This is called by BatchRender for the first member of every group before drawing. It calls the function specified by
		setGroupBeginFunction(). 
		@see groupEnd()
	*/
	virtual void groupEnd()
	{
		if( groupend )
			groupend();
	}

	//! Translate
	/*!
		Adds the given vector to each vertex in the geometry, translating the geometery.
		\f$ v_n = v_n + t \f$
	*/
	inline void translate( const Vector2d& _t )
	{
		BOOST_FOREACH( Vertex& v, vertices )
		{
			v.position += _t;
		}
	}

	//! Scale
	/*!
		Multiples every vertex in the geometry the given vector linearly.
		\f$ v_n = ( v_{nx} + s_x , v_{ny} + s_y ) \f$
	*/
	inline void scale( const Vector2d& _s )
	{
		BOOST_FOREACH( Vertex& v, vertices )
		{
			v.position.setX( v.position.getX() * _s.getX() );
			v.position.setY( v.position.getY() * _s.getY() );
		}
	}

	//! Rotate
	/*!
		Rotates each vertex by the given rotation matrix.
		\f$ v_n = v_n * m \f$
	*/
	inline void rotate( const RotationMatrix& _m )
	{
		BOOST_FOREACH( Vertex& v, vertices )
		{
			v.position *= _m;
		}
	}

	//! Sets the color on all vertices.
	inline void colorize( const Color& _c )
	{
		BOOST_FOREACH( Vertex& v, vertices )
		{
			v.color = _c;
		}
	}

	//! Define vertices using a Polygon.
	/*!
		Sets our vertices equal to the given Polygon. This is used by the polygon constructor.
		\note No colors or texture coords are set.
	*/
	inline void fromPolygon ( const Polygon& rhs )
	{
		setPrimitiveType( GL_TRIANGLES ); //primive type must be triangles.
		update();
		vertices.clear();
		//simple vertex to triangle expansion.
		if( rhs.getVertexCount() > 2 )
		{
			for( unsigned int i = 0; i < rhs.getVertexCount(); ++i )
			{
				vertices.push_back( rhs.getPosition() );
				vertices.push_back( rhs.getPosition() + rhs.getVertex(i) );
				vertices.push_back( rhs.getPosition() + rhs.getVertex(i+1) );
			}
		}
	}

	//! Define vertices using a Rectangle.
	/*!
		Sets our vertices equal to the given Rectangle. This is used by the Rectangle constructor.
	*/
	inline void fromRectangle( const Rectangle &rhs )
	{
		setPrimitiveType( GL_QUADS ); //primive type must be quads.
		update();
		vertices.clear();
		vertices.push_back( Vertex( Vector2d(0,0), Color(), TextureCoords(0,0) ) );
		vertices.push_back( Vertex( Vector2d(0, rhs.getDimensions().getY() ), Color(), TextureCoords(0,1) ) );
		vertices.push_back( Vertex( rhs.getDimensions(), Color(), TextureCoords(1,1) ) );
		vertices.push_back( Vertex( Vector2d( rhs.getDimensions().getX(), 0 ), Color(), TextureCoords(1,0) ) );
		translate( rhs.getPosition() );
	}

protected:

	//! Renderer
	BatchRenderer& renderer;

	//! Primitive type
	/*
		This is the OpenGL primitive type of this piece of geometry. It defaults to
		GL_QUADS, but may be GL_TRIANGLES, GL_LINES, GL_POINTS, etc.
	*/
	TrackingInvariant<unsigned int> primitivetype;

	//! Texture ID
	/*
		The invariant of the Texture's ID. This is 0 if this geometry has no texture.
	*/
	TrackingInvariant< unsigned > textureid;

	//! Texture
	boost::shared_ptr<Texture> texture;

	//! Group ID.
	/*
		The Group identifier of this geometry. Geometries of the same group usually have the same properties and
		share render states that are set and unset via the groupEnd() and groupBegin() functions. All geometry of
		the same group id should have the same groupEnd() and groupBegin() functions.
	*/
	TrackingInvariant< signed int > groupid;

	//! Depth
	TrackingInvariant< float > depth;

	//! Enabled
	bool enabled;

	//! Vertices
	std::vector< Vertex > vertices;

	//! Immediate
	/*
		If geometry is immediate, it is dropped right after it is drawn, so it only stays for one frame.
	*/
	bool immediate;

	//! Group Begin Function
	/*
		This function is called by the renderer on the first geometry of any given group before that group is drawn. 
		This should establish common renderstate of all members of the group (such as a new blending mode). All 
		geometry of the same type should have the same group functions.
	*/
	boost::function< void() > groupbegin;

	//! Group End Function
	/*
		This function is called by the renderer on the first geometry of any given group after that group is drawn. 
		This should restore anything set by the group begin function (such as restoring a blending mode). All 
		geometry of the same type should have the same group functions.
	*/
	boost::function< void() > groupend;

	//! Private Constructor
	BatchGeometry(BatchRenderer& _r, unsigned int _p = GL_QUADS, boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>(), signed int _g = 0, float _d = 0.0f )
		: Droppable(), renderer(_r), primitivetype(_p), vertices(), textureid( _t ? _t->getTextureId() : 0 ), texture(_t), groupid(_g), depth(_d), enabled(true), immediate(false), groupbegin(), groupend()
	{
#ifdef DEBUG_BATCHGEOMETRY
		std::cout<<"\nBatch Geometry Created: "<<_r<<", "<<_p<<", "<<_t.get()<<", "<<_g<<", "<<_d<<std::endl;
#endif
	}


};

} //namespace phoniex

#endif // __PHBATCHGEOMETRY_H__
