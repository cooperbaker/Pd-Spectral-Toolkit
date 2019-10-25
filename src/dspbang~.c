//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  dspbang~.c
//
//  Outputs a bang when dsp is turned on
//
//  Created by Cooper Baker on 6/5/12.
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
// dspbang_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* dspbang_class;


//------------------------------------------------------------------------------
// dspbang - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct dspbang
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in dspbang_tilde_setup
    t_float inlet_1;

    // pointer to the outlet
    t_outlet* outlet_1;

} t_dspbang;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static void   dspbang_dsp         ( t_dspbang* object, t_signal **sig );
static void*  dspbang_new         ( void );
void          dspbang_tilde_setup ( void );


//------------------------------------------------------------------------------
// dspbang_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void dspbang_dsp( t_dspbang* object, t_signal **sig )
{
    outlet_bang( object->outlet_1 );
}


//------------------------------------------------------------------------------
// dspbang_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* dspbang_new( void )
{
    // create a pointer to this object
    t_dspbang* object = ( t_dspbang* )pd_new( dspbang_class );

    // create a new outlet for this object
    object->outlet_1 = outlet_new( &object->object, gensym( "bang" ) );

    return object;
}


//------------------------------------------------------------------------------
// dspbang_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void dspbang_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    dspbang_class = class_new( gensym( "dspbang~" ), ( t_newmethod )dspbang_new, 0, sizeof( t_dspbang ), CLASS_NOINLET, 0 );

    // installs dspbang_dsp so that it will be called when dsp is turned on
    class_addmethod( dspbang_class, ( t_method )dspbang_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
