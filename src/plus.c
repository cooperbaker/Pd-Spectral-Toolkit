//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  plus.c
//
//  Example Object
//  Sums floats from two inlets to one outlet and also accepts an argument
//
//  Created by Cooper Baker on 3/28/12.
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
// plus_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* plus_class;


//------------------------------------------------------------------------------
// plus - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct plus
{
      // this object - must always be first variable in struct
      t_object object;

      // pointer to an outlet
      t_outlet* outlet;

      // floats to hold inlets' values
      t_float value_0;
      t_float value_1;

} t_plus;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  plus_bang    ( t_plus* object );
void  plus_float_0 ( t_plus* object, t_floatarg number );
void  plus_float_1 ( t_plus* object, t_floatarg number );
void* plus_new     ( t_floatarg arg );
void  plus_setup   ( void );


//------------------------------------------------------------------------------
// plus_bang - bang handler
//------------------------------------------------------------------------------
void plus_bang( t_plus* object )
{
    // calculate the sum
    float sum = object->value_0 + object->value_1;

    // ouput the sum through this object's outlet
    outlet_float( object->outlet, sum );
}


//------------------------------------------------------------------------------
// plus_float_0 - inlet 0 float handler
//------------------------------------------------------------------------------
void plus_float_0( t_plus* object, t_floatarg number )
{
    // store float value in object variable "value_0"
    object->value_0 = number;

    // call bang - cause output only when inlet 0 receives a message
    plus_bang( object );
}


//------------------------------------------------------------------------------
// plus_float_1 - inlet 1 float handler
//------------------------------------------------------------------------------
void plus_float_1( t_plus* object, t_floatarg number )
{
    // store float value in object variable "value_1"
    object->value_1 = number;
}


//------------------------------------------------------------------------------
// plus_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* plus_new( t_floatarg arg )
{
    // declare a pointer to this object
    t_plus* object;

    // generate a new object and save its pointer in "object"
    object = ( t_plus* )pd_new( plus_class );

    // create a new inlet
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "float_1" ) );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "float" ) );

    // store the initialization argument for later use
    object->value_1 = arg;

    // return the pointer to this object
    return ( void* )object;
}


//------------------------------------------------------------------------------
// plus setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void plus_setup( void )
{
    // create a new class and assign its pointer to plus_class
    // the A_DEFFLOAT indicates this object expects a float argument
    // which will default to zero if not supplied
    plus_class = class_new( gensym( "plus" ), ( t_newmethod )plus_new, 0, sizeof( t_plus ), 0, A_DEFFLOAT, 0 );

    // add a method to this object for handling floats in inlet one
    class_addmethod( plus_class, ( t_method )plus_float_1, gensym( "float_1" ), A_FLOAT, 0 );

    // add float handler
    class_addfloat( plus_class, plus_float_0 );

    // add bang handler
    class_addbang( plus_class, plus_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
