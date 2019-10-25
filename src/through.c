//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  through.c
//
//  Example object
//  Passes a float from inlet to outlet
//
//  Created by Cooper Baker on 3/29/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// m_pd.h - main header for Pd
//------------------------------------------------------------------------------
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"


//------------------------------------------------------------------------------
// through_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* through_class;


//------------------------------------------------------------------------------
// through - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct through
{
    // this object - must always be first variable in struct
    t_object object;

    // float variable to hold input value
    t_float value;

    // pointer to an outlet
    t_outlet* outlet;

} t_through;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  through_bang  ( t_through* object );
void  through_float ( t_through* object, t_floatarg number );
void* through_new   ( void );
void  through_setup ( void );


//------------------------------------------------------------------------------
// through_bang - causes output
//------------------------------------------------------------------------------
void through_bang( t_through* object )
{
    // send stored value to the object's outlet
    // objects should always respond to bangs this way
    // so that they conform to Pd's object-graph message ordering scheme
    outlet_float( object->outlet, object->value );
}


//------------------------------------------------------------------------------
// through_float - default float handler
//------------------------------------------------------------------------------
void through_float( t_through* object, t_floatarg number )
{
    // store number in to the object's "value" variable
    object->value = number;

    through_bang( object );
}


//------------------------------------------------------------------------------
// through_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* through_new( void )
{
    // declare a pointer to this class
    t_through* object;

    // generate a new object and save its pointer in "object"
    object = ( t_through* )pd_new( through_class );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "float" ) );

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// through setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void through_setup( void )
{
    // create a new class and assign its pointer to through_class
    through_class = class_new( gensym( "through" ), ( t_newmethod )through_new, 0, sizeof( t_through ), 0, 0 );

    // add default float handler
    class_addfloat( through_class, through_float );

    // add default bang handler
    class_addbang( through_class, through_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
