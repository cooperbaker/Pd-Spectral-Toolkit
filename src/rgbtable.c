//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  rgbtable.c
//
//  Outputs red, green, and blue values based on an input value
//
//  Created by Cooper on 9/22/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// headers
//------------------------------------------------------------------------------
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define COLOR_TABLE_SIZE 1000000


//------------------------------------------------------------------------------
// rgbtable_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* rgbtable_class;


//------------------------------------------------------------------------------
// SCHEME_ID - enumerated color scheme names
//------------------------------------------------------------------------------
enum SCHEME_ID
{
    RGB,
    PURPLE_YELLOW,
    BLUE_GREEN,
    AMBER,
    RED,
    GREEN,
    BLUE,
    GREY
};


//------------------------------------------------------------------------------
// rgbtable - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct rgbtable
{
    // this object - must always be first variable in struct
    t_object object;

    // float variable to hold input value
    t_float value;

    // id of the selected color scheme
    t_int scheme_id;

    // pointers to outlets
    t_outlet* outlet_r;
    t_outlet* outlet_g;
    t_outlet* outlet_b;

    // meta pointers to the color tables
    t_float* r;
    t_float* g;
    t_float* b;

    // pointers to the color tables
    t_float* r_rgb;
    t_float* g_rgb;
    t_float* b_rgb;

    t_float* r_py;
    t_float* g_py;
    t_float* b_py;

    t_float* r_bg;
    t_float* g_bg;
    t_float* b_bg;

    t_float* r_a;
    t_float* g_a;
    t_float* b_a;

    t_float* r_r;
    t_float* g_r;
    t_float* b_r;

    t_float* r_g;
    t_float* g_g;
    t_float* b_g;

    t_float* r_b;
    t_float* g_b;
    t_float* b_b;

    t_float* r_gr;
    t_float* g_gr;
    t_float* b_gr;

} t_rgbtable;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void        rgbtable_bang       ( t_rgbtable* object );
void        rgbtable_float      ( t_rgbtable* object, t_floatarg number );
void        rgbtable_set_scheme ( t_rgbtable* object );
void*       rgbtable_new        ( t_symbol* selector, t_int items, t_atom* list );
static void rgbtable_free       ( t_rgbtable* object );
void        rgbtable_setup      ( void );


//------------------------------------------------------------------------------
// rgbtable_bang - causes output
//------------------------------------------------------------------------------
void rgbtable_bang( t_rgbtable* object )
{
    // calculate table index
    t_int index = object->value *= COLOR_TABLE_SIZE - 1;

    // output rgb values
    outlet_float( object->outlet_r, object->r[ index ] );
    outlet_float( object->outlet_g, object->g[ index ] );
    outlet_float( object->outlet_b, object->b[ index ] );
}


//------------------------------------------------------------------------------
// rgbtable_float - default float handler
//------------------------------------------------------------------------------
void rgbtable_float( t_rgbtable* object, t_floatarg number )
{
    // store number in to the object's "value" variable
    object->value = Clip( number, 0, 1 );

    rgbtable_bang( object );
}


//------------------------------------------------------------------------------
// rgbtable_message_parse - parses incoming messages to set color tables
//------------------------------------------------------------------------------
void rgbtable_message_parse( t_rgbtable* object, t_symbol* selector, t_int items, t_atom* list )
{
    // get selector string
    const char* message = selector->s_name;

    // rgb
    if( StringMatch( message, "rgb" ) )
    {
        object->scheme_id = RGB;
    }

    // purple yellow
    else if( StringMatch( message, "purple-yellow" ) )
    {
        object->scheme_id = PURPLE_YELLOW;
    }

    // blue green
    else if( StringMatch( message, "blue-green" ) )
    {
        object->scheme_id = BLUE_GREEN;
    }

    // amber
    else if( StringMatch( message, "amber" ) )
    {
        object->scheme_id = AMBER;
    }

    // red
    else if( StringMatch( message, "red" ) )
    {
        object->scheme_id = RED;
    }

    // green
    else if( StringMatch( message, "green" ) )
    {
        object->scheme_id = GREEN;
    }

    // blue
    else if( StringMatch( message, "blue" ) )
    {
        object->scheme_id = BLUE;
    }

    // grey
    else if( StringMatch( message, "grey" ) )
    {
        object->scheme_id = GREY;
    }

    // unknown
    else
    {
        pd_error( object, "rgbtable: unknown color scheme name" );
    }

    rgbtable_set_scheme( object );
}

//------------------------------------------------------------------------------
// rgbtable_set_scheme - sets color scheme
//------------------------------------------------------------------------------
void rgbtable_set_scheme( t_rgbtable* object )
{
    switch( object->scheme_id )
    {
        case RGB            :   object->r = object->r_rgb;
                                object->g = object->g_rgb;
                                object->b = object->b_rgb;
                                break;

        case PURPLE_YELLOW  :   object->r = object->r_py;
                                object->g = object->g_py;
                                object->b = object->b_py;
                                break;

        case BLUE_GREEN     :   object->r = object->r_bg;
                                object->g = object->g_bg;
                                object->b = object->b_bg;
                                break;

        case AMBER          :   object->r = object->r_a;
                                object->g = object->g_a;
                                object->b = object->b_a;
                                break;

        case RED            :   object->r = object->r_r;
                                object->g = object->g_r;
                                object->b = object->b_r;
                                break;

        case GREEN          :   object->r = object->r_g;
                                object->g = object->g_g;
                                object->b = object->b_g;
                                break;

        case BLUE           :   object->r = object->r_b;
                                object->g = object->g_b;
                                object->b = object->b_b;
                                break;

        case GREY           :   object->r = object->r_b;
                                object->g = object->g_b;
                                object->b = object->b_b;
                                break;
    }
}


//------------------------------------------------------------------------------
// rgbtable_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* rgbtable_new( t_symbol* selector, t_int items, t_atom* list )
{
    // declare a pointer to this class
    t_rgbtable* object;

    // generate a new object and save its pointer in "object"
    object = ( t_rgbtable* )pd_new( rgbtable_class );

    // generate outlets and save their pointers
    object->outlet_r = outlet_new( &object->object, gensym( "float" ) );
    object->outlet_g = outlet_new( &object->object, gensym( "float" ) );
    object->outlet_b = outlet_new( &object->object, gensym( "float" ) );

    // initialize the pointers
    //--------------------------------------------------------------------------

    // meta pointers
    object->r = NULL;
    object->g = NULL;
    object->b = NULL;

     // rgb
    object->r_rgb = NULL;
    object->g_rgb = NULL;
    object->b_rgb = NULL;

    // purple yellow
    object->r_py = NULL;
    object->g_py = NULL;
    object->b_py = NULL;

    // blue green
    object->r_bg = NULL;
    object->g_bg = NULL;
    object->b_bg = NULL;

    // amber
    object->r_a = NULL;
    object->g_a = NULL;
    object->b_a = NULL;

    // red
    object->r_r = NULL;
    object->g_r = NULL;
    object->b_r = NULL;

    // green
    object->r_g = NULL;
    object->g_g = NULL;
    object->b_g = NULL;

    // blue
    object->r_b = NULL;
    object->g_b = NULL;
    object->b_b = NULL;

    // grey
    object->r_gr = NULL;
    object->g_gr = NULL;
    object->b_gr = NULL;


    // allocate memory for the color tables
    //--------------------------------------------------------------------------

    // rgb
    object->r_rgb = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_rgb = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_rgb = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // purple yellow
    object->r_py = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_py = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_py = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // blue green
    object->r_bg = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_bg = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_bg = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // amber
    object->r_a = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_a = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_a = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // red
    object->r_r = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_r = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_r = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // green
    object->r_g = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_g = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_g = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // blue
    object->r_b = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_b = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_b = calloc( COLOR_TABLE_SIZE, sizeof( float ) );

    // grey
    object->r_gr = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->g_gr = calloc( COLOR_TABLE_SIZE, sizeof( float ) );
    object->b_gr = calloc( COLOR_TABLE_SIZE, sizeof( float ) );



    // color curve calculation variables
    t_int i;
    t_float x;

    // iterate through the tables
    for( i = 0 ; i < COLOR_TABLE_SIZE ; ++i )
    {
        // calculate x from 0 to 1
        x = ( float )i / COLOR_TABLE_SIZE;

        // calculate the color curves and store into tables
        //----------------------------------------------------------------------

        // rgb
        object->r_rgb[ i ] = 0.5 * ( 1.0 - Cosine( C_PI * x * x ) );
        object->g_rgb[ i ] = 0.5 * ( 1.0 - Cosine( C_2_PI * x * x ) );
        object->b_rgb[ i ] = ( 1.0 - ( 0.5 * ( 1.0 - Cosine( C_PI * x ) ) ) ) * Power( x, 0.75 ) * 3;

        // purple yellow
        object->r_py[ i ] = 0.5 * ( 1.0 - Cosine( C_PI * Power( x, 1.5 ) ) );
        object->g_py[ i ] = x * x * x * x;
        object->b_py[ i ] = ( 1.0 - ( 0.5 * ( 1.0 - Cosine( C_PI * x * x ) ) ) ) * Power( x, 0.75 ) * 1.25;

        // blue green
        object->r_bg[ i ] = x * x * x * x;
        object->g_bg[ i ] = 0.5 * ( 1.0 - Cosine( C_PI * Power( x, 1.5 ) ) );
        object->b_bg[ i ] = ( 1.0 - ( 0.5 * ( 1.0 - Cosine( C_PI * x * x ) ) ) ) * Power( x, 0.75 ) * 1.25;

        // amber
        object->r_a[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x ) ) );
        object->g_a[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x ) ) );
        object->b_a[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x * x ) ) );

        // red
        object->r_r[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x ) ) );
        object->g_r[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x * x * x ) ) );
        object->b_r[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x * x * x ) ) );

        // green
        object->r_g[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x * x ) ) );
        object->g_g[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x ) ) );
        object->b_g[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x ) ) );

        // blue
        object->r_b[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x * x ) ) );
        object->g_b[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x * x ) ) );
        object->b_b[ i ] = ( 0.5 * ( 1.0 - Cosine( C_PI * x ) ) );

        // grey
        object->r_gr[ i ] = x;
        object->g_gr[ i ] = x;
        object->b_gr[ i ] = x;
    }

    // set default color scheme id
    object->scheme_id = RGB;

    // set the color scheme
    rgbtable_set_scheme( object );

    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            t_symbol init_selector;

            init_selector.s_name = list[ 0 ].a_w.w_symbol->s_name;

            rgbtable_message_parse( object, &init_selector, 0, NULL );
        }
        else
        {
            // notify of invalid arguments
            pd_error( object, "rgbtable: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        // notify of extra arguments
        pd_error( object, "rgbtable: extra arguments ignored" );
    }

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// rgbtable_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
void rgbtable_free( t_rgbtable* object )
{
    // if memory is allocated
    // {
    //    deallocate the memory
    //    set the memory pointer to null
    // }

    // rgb
    if( object->r_rgb )
    {
        free( object->r_rgb );
        object->r_rgb = NULL;
    }

    if( object->g_rgb )
    {
        free( object->g_rgb );
        object->g_rgb = NULL;
    }

    if( object->b_rgb )
    {
        free( object->b_rgb );
        object->b_rgb = NULL;
    }

    // purple yellow
    if( object->r_py )
    {
        free( object->r_py );
        object->r_py = NULL;
    }

    if( object->g_py )
    {
        free( object->g_py );
        object->g_py = NULL;
    }

    if( object->b_py )
    {
        free( object->b_py );
        object->b_py = NULL;
    }

    // blue green
    if( object->r_bg )
    {
        free( object->r_bg );
        object->r_bg = NULL;
    }

    if( object->g_bg )
    {
        free( object->g_bg );
        object->g_bg = NULL;
    }

    if( object->b_bg )
    {
        free( object->b_bg );
        object->b_bg = NULL;
    }

    // amber
    if( object->r_a )
    {
        free( object->r_a );
        object->r_a = NULL;
    }

    if( object->g_a )
    {
        free( object->g_a );
        object->g_a = NULL;
    }

    if( object->b_a )
    {
        free( object->b_a );
        object->b_a = NULL;
    }

    // red
    if( object->r_r )
    {
        free( object->r_r );
        object->r_r = NULL;
    }

    if( object->g_r )
    {
        free( object->g_r );
        object->g_r = NULL;
    }

    if( object->b_r )
    {
        free( object->b_r );
        object->b_r = NULL;
    }

    // green
    if( object->r_g )
    {
        free( object->r_g );
        object->r_g = NULL;
    }

    if( object->g_g )
    {
        free( object->g_g );
        object->g_g = NULL;
    }

    if( object->b_g )
    {
        free( object->b_g );
        object->b_g = NULL;
    }

    // blue
    if( object->r_b )
    {
        free( object->r_b );
        object->r_b = NULL;
    }

    if( object->g_b )
    {
        free( object->g_b );
        object->g_b = NULL;
    }

    if( object->b_b )
    {
        free( object->b_b );
        object->b_b = NULL;
    }

}


//------------------------------------------------------------------------------
// rgbtable setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void rgbtable_setup( void )
{
    // create a new class and assign its pointer to rgbtable_class
    rgbtable_class = class_new( gensym( "rgbtable" ), ( t_newmethod )rgbtable_new, ( t_method )rgbtable_free, sizeof( t_rgbtable ), 0, A_GIMME, 0 );

    // add default float handler
    class_addfloat( rgbtable_class, rgbtable_float );

    // add default bang handler
    class_addbang( rgbtable_class, rgbtable_bang );

    // add message handler
    class_addmethod( rgbtable_class, ( t_method )rgbtable_message_parse, gensym( "anything" ), A_GIMME, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
