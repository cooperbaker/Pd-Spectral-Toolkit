//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  countwrap.c
//
//  Counts between limits using an arbitrary increment and wraps overflow
//
//  Created by Cooper Baker on 5/22/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// headers
//------------------------------------------------------------------------------

// main header for pd
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// countwrap_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* countwrap_class;


//------------------------------------------------------------------------------
// countwrap - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct countwrap
{
    // this object - must always be first variable in struct
    t_object object;

    // the current count
    t_float count;

    // the counting increment
    t_float increment;

    // the minimum count
    t_float minimum;

    // the maximum count
    t_float maximum;

    // pointer to an outlet
    t_outlet* outlet;

} t_countwrap;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  countwrap_bang      ( t_countwrap* object );
void* countwrap_new       ( t_symbol* selector, t_int items, t_atom* list );
void  countwrap_setup     ( void );
void  countwrap_count     ( t_countwrap* object, t_floatarg count );
void  countwrap_increment ( t_countwrap* object, t_floatarg increment );
void  countwrap_minimum   ( t_countwrap* object, t_floatarg minimum );
void  countwrap_maximum   ( t_countwrap* object, t_floatarg maximum );


//------------------------------------------------------------------------------
// countwrap_bang - causes output
//------------------------------------------------------------------------------
void countwrap_bang( t_countwrap* object )
{
    // switch minimum/maximum if necessary
    if( object->minimum > object->maximum )
    {
        t_float temp = object->maximum;

        object->maximum = object->minimum;
        object->minimum = temp;
    }

    if( ( object->minimum == object->maximum ) || ( object->increment == 0 ) )
    {
        // do not count if no counting will happen
    }
    else
    {
        // calculate counting range
        t_float range = object->maximum - object->minimum;

        // increment the count
        object->count += object->increment;

        // wrap if count is below minimum
        while( object->count <= object->minimum )
        {
            object->count += range;
        }

        // wrap if count is above maximum
        while( object->count >= object->maximum )
        {
            object->count -= range;
        }
    }

    // output the count
    outlet_float( object->outlet, object->count );
}


//------------------------------------------------------------------------------
// countwrap_count - sets current count value
//------------------------------------------------------------------------------
void countwrap_count( t_countwrap* object, t_floatarg count )
{
    object->count = count;
}


//------------------------------------------------------------------------------
// countwrap_increment - sets increment value
//------------------------------------------------------------------------------
void countwrap_increment( t_countwrap* object, t_floatarg increment )
{
    object->increment = increment;
}


//------------------------------------------------------------------------------
// countwrap_minimum - sets minimum count value
//------------------------------------------------------------------------------
void countwrap_minimum( t_countwrap* object, t_floatarg minimum )
{
    object->minimum = minimum;
}


//------------------------------------------------------------------------------
// countwrap_maximum - sets maximum count value
//------------------------------------------------------------------------------
void countwrap_maximum( t_countwrap* object, t_floatarg maximum )
{
    object->maximum = maximum;
}


//------------------------------------------------------------------------------
// countwrap_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* countwrap_new( t_symbol* selector, t_int items, t_atom* list )
{
    // declare a pointer to this class
    t_countwrap* object;

    // generate a new object and save its pointer in "object"
    object = ( t_countwrap* )pd_new( countwrap_class );

    // create some new inlets ( ordered left to right )
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "count"     ) );
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "increment" ) );
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "minimum"   ) );
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "maximum"   ) );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "float" ) );

    // initialize variables
    object->count     = 0;
    object->increment = 1;
    object->minimum   = 0;
    object->maximum   = C_FLOAT_MAX;

    // parse arguments and store their values
    if( items > 3 )
    {
        object->count     = list[ 0 ].a_w.w_float;
        object->increment = list[ 1 ].a_w.w_float;
        object->minimum   = list[ 2 ].a_w.w_float;
        object->maximum   = list[ 3 ].a_w.w_float;
    }
    else if( items > 2 )
    {
        object->increment = list[ 0 ].a_w.w_float;
        object->minimum   = list[ 1 ].a_w.w_float;
        object->maximum   = list[ 2 ].a_w.w_float;
    }
    else if( items > 1 )
    {
        object->minimum   = list[ 0 ].a_w.w_float;
        object->maximum   = list[ 1 ].a_w.w_float;
    }
    else if( items > 0 )
    {
        object->increment = list[ 0 ].a_w.w_float;
    }

    // notify of extra arguments
    if( items > 4 )
    {
        pd_error( object, "countwrap: initialization: extra arguments ignored" );
    }


    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// countwrap_setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void countwrap_setup( void )
{
    // create a new class and assign its pointer to countwrap_class
    countwrap_class = class_new( gensym( "countwrap" ), ( t_newmethod )countwrap_new, 0, sizeof( t_countwrap ), 0, A_GIMME, 0 );

    // add float inlets and associate their handlers
    class_addmethod( countwrap_class, ( t_method )countwrap_count,     gensym( "count"     ), A_FLOAT, 0 );
    class_addmethod( countwrap_class, ( t_method )countwrap_increment, gensym( "increment" ), A_FLOAT, 0 );
    class_addmethod( countwrap_class, ( t_method )countwrap_minimum,   gensym( "minimum"   ), A_FLOAT, 0 );
    class_addmethod( countwrap_class, ( t_method )countwrap_maximum,   gensym( "maximum"   ), A_FLOAT, 0 );

    // add default bang handler
    class_addbang( countwrap_class, countwrap_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
