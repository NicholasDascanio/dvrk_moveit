/*
* Copyright 2006 Sony Computer Entertainment Inc.
*
* Licensed under the MIT Open Source License, for details please see license.txt or the website
* http://www.opensource.org/licenses/mit-license.php
*
*/ 

#ifndef __dom141Library_lights_h__
#define __dom141Library_lights_h__

#include <dae/daeDocument.h>
#include <1.4/dom/domTypes.h>
#include <1.4/dom/domElements.h>

#include <1.4/dom/domAsset.h>
#include <1.4/dom/domLight.h>
#include <1.4/dom/domExtra.h>

class DAE;
namespace ColladaDOM141 {

/**
 * The library_lights element declares a module of light elements.
 */
class domLibrary_lights : public daeElement
{
public:
	virtual COLLADA_TYPE::TypeEnum getElementType() const { return COLLADA_TYPE::LIBRARY_LIGHTS; }
	static daeInt ID() { return 720; }
	virtual daeInt typeID() const { return ID(); }
protected:  // Attributes
/**
 *  The id attribute is a text string containing the unique identifier of
 * this element.  This value must be unique within the instance document.
 * Optional attribute. 
 */
	xsID attrId;
/**
 *  The name attribute is the text string name of this element. Optional attribute.
 */
	xsNCName attrName;

protected:  // Elements
/**
 *  The library_lights element may contain an asset element.  @see domAsset
 */
	domAssetRef elemAsset;
/**
 *  There must be at least one light element.  @see domLight
 */
	domLight_Array elemLight_array;
/**
 *  The extra element may appear any number of times.  @see domExtra
 */
	domExtra_Array elemExtra_array;

public:	//Accessors and Mutators
	/**
	 * Gets the id attribute.
	 * @return Returns a xsID of the id attribute.
	 */
	xsID getId() const { return attrId; }
	/**
	 * Sets the id attribute.
	 * @param atId The new value for the id attribute.
	 */
	void setId( xsID atId ) { *(daeStringRef*)&attrId = atId; _validAttributeArray[0] = true; 
		if( _document != NULL ) _document->changeElementID( this, attrId );
	}

	/**
	 * Gets the name attribute.
	 * @return Returns a xsNCName of the name attribute.
	 */
	xsNCName getName() const { return attrName; }
	/**
	 * Sets the name attribute.
	 * @param atName The new value for the name attribute.
	 */
	void setName( xsNCName atName ) { *(daeStringRef*)&attrName = atName; _validAttributeArray[1] = true; }

	/**
	 * Gets the asset element.
	 * @return a daeSmartRef to the asset element.
	 */
	const domAssetRef getAsset() const { return elemAsset; }
	/**
	 * Gets the light element array.
	 * @return Returns a reference to the array of light elements.
	 */
	domLight_Array &getLight_array() { return elemLight_array; }
	/**
	 * Gets the light element array.
	 * @return Returns a constant reference to the array of light elements.
	 */
	const domLight_Array &getLight_array() const { return elemLight_array; }
	/**
	 * Gets the extra element array.
	 * @return Returns a reference to the array of extra elements.
	 */
	domExtra_Array &getExtra_array() { return elemExtra_array; }
	/**
	 * Gets the extra element array.
	 * @return Returns a constant reference to the array of extra elements.
	 */
	const domExtra_Array &getExtra_array() const { return elemExtra_array; }
protected:
	/**
	 * Constructor
	 */
	domLibrary_lights(DAE& dae) : daeElement(dae), attrId(), attrName(), elemAsset(), elemLight_array(), elemExtra_array() {}
	/**
	 * Destructor
	 */
	virtual ~domLibrary_lights() {}
	/**
	 * Overloaded assignment operator
	 */
	virtual domLibrary_lights &operator=( const domLibrary_lights &cpy ) { (void)cpy; return *this; }

public: // STATIC METHODS
	/**
	 * Creates an instance of this class and returns a daeElementRef referencing it.
	 * @return a daeElementRef referencing an instance of this object.
	 */
	static DLLSPEC daeElementRef create(DAE& dae);
	/**
	 * Creates a daeMetaElement object that describes this element in the meta object reflection framework.
	 * If a daeMetaElement already exists it will return that instead of creating a new one. 
	 * @return A daeMetaElement describing this COLLADA element.
	 */
	static DLLSPEC daeMetaElement* registerElement(DAE& dae);
};


} // ColladaDOM141
#endif
