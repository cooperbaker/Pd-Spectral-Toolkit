//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  outlets.c
//
//  Example Object
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
// outlets_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* outlets_class;


//------------------------------------------------------------------------------
// outlets - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct outlets
{
    // this object - must always be first variable in struct
    t_object object;

    // float to hold input value
    t_float value;

    // pointers to the outlets
    t_outlet* outlet_0;
    t_outlet* outlet_1;
    t_outlet* outlet_2;
    t_outlet* outlet_3;

} t_outlets;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  outlets_bang  ( t_outlets* object );
void  outlets_float ( t_outlets* object, t_floatarg number );
void* outlets_new   ( void );
void  outlets_setup ( void );


//------------------------------------------------------------------------------
// outlets_bang - causes output
//------------------------------------------------------------------------------
void outlets_bang( t_outlets* object )
{
    outlet_float( object->outlet_0, object->value );
    outlet_float( object->outlet_1, object->value );
    outlet_float( object->outlet_2, object->value );
    outlet_float( object->outlet_3, object->value );
}


//------------------------------------------------------------------------------
// outlets_float - default float handler
//------------------------------------------------------------------------------
void outlets_float( t_outlets* object, t_floatarg number )
{
    // send "number" to the object's outlet
    object->value = number;

    // call outlets_bang to cause output
    outlets_bang( object );
}


//------------------------------------------------------------------------------
// outlets_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* outlets_new( void )
{
    // declare a pointer to this class
    t_outlets* object;

    // generate a new object and save its pointer in "object"
    object = ( t_outlets* )pd_new( outlets_class );

    // generate some new outlets and save their pointers
    object->outlet_0 = outlet_new( &object->object, gensym( "float" ) );
    object->outlet_1 = outlet_new( &object->object, gensym( "float" ) );
    object->outlet_2 = outlet_new( &object->object, gensym( "float" ) );
    object->outlet_3 = outlet_new( &object->object, gensym( "float" ) );

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// outlets setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void outlets_setup( void )
{
    // create a new class and assign its pointer to outlets_class
    outlets_class = class_new( gensym( "outlets" ), ( t_newmethod )outlets_new, 0, sizeof( t_outlets ), 0, 0 );

    // add default float handler
    class_addfloat( outlets_class, outlets_float );

    // add default bang handler
    class_addbang( outlets_class, outlets_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
