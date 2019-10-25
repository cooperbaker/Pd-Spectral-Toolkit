//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  inlets.c
//
//  Example Object
//  Sums floats from four inlets to one outlet
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
// inlets_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* inlets_class;


//------------------------------------------------------------------------------
// inlets - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct inlets
{
    // this object - must always be first variable in struct
    t_object object;

    // float variables to hold inlets' values
    t_float value_0;
    t_float value_1;
    t_float value_2;
    t_float value_3;

    // pointer to an outlet
    t_outlet* outlet;

} t_inlets;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  inlets_float ( t_inlets* object, t_floatarg number );
void* inlets_new   ( void );
void  inlets_setup ( void );


//------------------------------------------------------------------------------
// inlets_bang - causes output
//------------------------------------------------------------------------------
void inlets_bang( t_inlets* object )
{
    // add the stored values together
    float sum = object->value_0 + object->value_1 + object->value_2 + object->value_3;

    // output the summed values
    outlet_float( object->outlet, sum );
}


//------------------------------------------------------------------------------
// inlets_float_0 - inlet 0 float handler
//------------------------------------------------------------------------------
void inlets_float_0( t_inlets* object, t_floatarg number )
{
    // store number in the corresponding value_x variable
    object->value_0 = number;

    // call inlets_bang to cause output
    inlets_bang( object );
}


//------------------------------------------------------------------------------
// inlets_float_1 - inlet 1 float handler
//------------------------------------------------------------------------------
void inlets_float_1( t_inlets* object, t_floatarg number )
{
    // store number in the corresponding value_x variable
    object->value_1 = number;
}


//------------------------------------------------------------------------------
// inlets_float_2 - inlet 2 float handler
//------------------------------------------------------------------------------
void inlets_float_2( t_inlets* object, t_floatarg number )
{
    // store number in the corresponding value_x variable
    object->value_2 = number;
}


//------------------------------------------------------------------------------
// inlets_float_3 - inlet 3 float handler
//------------------------------------------------------------------------------
void inlets_float_3( t_inlets* object, t_floatarg number )
{
    // store number in the corresponding value_x variable
    object->value_3 = number;
}


//------------------------------------------------------------------------------
// inlets_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* inlets_new( void )
{
    // declare a pointer to this class
    t_inlets* object;

    // generate a new object and save its pointer in "object"
    object = ( t_inlets* )pd_new( inlets_class );

    // create some new inlets ( ordered left to right )
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "float_1" ) );
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "float_2" ) );
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "float_3" ) );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "float" ) );

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// inlets setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void inlets_setup( void )
{
    // create a new class and assign its pointer to inlets_class
    inlets_class = class_new( gensym( "inlets" ), ( t_newmethod )inlets_new, 0, sizeof( t_inlets ), 0, 0 );

    // add float inlets and associate their handlers
    class_addmethod( inlets_class, ( t_method )inlets_float_1, gensym( "float_1" ), A_FLOAT, 0 );
    class_addmethod( inlets_class, ( t_method )inlets_float_2, gensym( "float_2" ), A_FLOAT, 0 );
    class_addmethod( inlets_class, ( t_method )inlets_float_3, gensym( "float_3" ), A_FLOAT, 0 );

    // add default float handler ( inlet 0 )
    class_addfloat( inlets_class, inlets_float_0 );

    // add bang handler
    class_addbang( inlets_class, ( t_method )inlets_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------

