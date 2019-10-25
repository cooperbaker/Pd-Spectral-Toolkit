//-------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  windower.c
//
//  Writes window functions into Pd arrays
//
//  Created by Cooper Baker on 5/8/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//-------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// m_pd.h - main header for Pd
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
// windower_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* windower_class;


//------------------------------------------------------------------------------
// WINDOW_ID - enumerated window names
//------------------------------------------------------------------------------
enum WINDOW_ID
{
    RECTANGLE,
    HANN,
    HAMMING,
    TUKEY,
    COSINE,
    LANCZOS,
    TRIANGLE,
    GAUSSIAN,
    BARTLETT_HANN,
    KAISER,
    NUTTALL,
    BLACKMAN,
    BLACKMAN_HARRIS,
    BLACKMAN_NUTTALL,
    POISSON,
    HANN_POISSON
};


//------------------------------------------------------------------------------
// windower - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct windower
{
    // this object - must always be first variable in struct
    t_object    object;

    // id of the window to write
    t_int       window_id;

    // name of the window to write
    t_float*    window;

    // name of the array being written into
    t_symbol*   array_name;

    // size of the array being written into
    t_int       array_size;

    // tukey window coefficient
    t_float     tukey_coeff;

    // gaussian window coefficient
    t_float     gaussian_coeff;

    // blackman window coefficient
    t_float     blackman_coeff;

    // kaiser window coefficient
    t_float     kaiser_coeff;

    // poisson window coefficient
    t_float     poisson_coeff;

    // hann-poisson window coefficient
    t_float     hann_poisson_coeff;

} t_windower;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

static void  windower_fill_array    ( t_windower* object );
void         windower_message_parse ( t_windower* object, t_symbol* selector, t_int items, t_atom* list );
void         windower_bang          ( t_windower* object );
static void* windower_new           ( t_symbol* selector, t_int items, t_atom* list );
void         windower_setup         ( void );


//------------------------------------------------------------------------------
// windower_fill_array - fills the associated array with the current window type
//------------------------------------------------------------------------------
static void windower_fill_array( t_windower* object )
{
    // array pointer
    t_garray* array;

    // array data pointer
    t_word* array_data;

    // array size
    int array_size;

    // array error checking flag
    t_int valid_array;

    // array size
    t_float N;

    // window function iterator
    t_float n;

    // array index iterator
    t_int index;

    // array value pointer
    t_float* value;

    // temp variables for window function calculation
    t_float a, a0, a1, a2, a3, temp;

    // check to make sure windower has an array name to work with
    //--------------------------------------------------------------------------
    if( object->array_name == NULL )
    {
        pd_error( object, "windower: no array name set" );
        return;
    }

    // check to make sure the array exists
    //--------------------------------------------------------------------------
    array = ( t_garray* )pd_findbyclass( object->array_name, garray_class );

    if( array == NULL )
    {
        pd_error( object, "windower: %s: no such array", object->array_name->s_name );
        return;
    }

    // check to make sure the array is valid
    //--------------------------------------------------------------------------
    valid_array = garray_getfloatwords( array, &array_size, &array_data );

    if( valid_array == 0 )
    {
        pd_error( object, "windower: %s: bad template for windower", object->array_name->s_name );
        return;
    }

    // save array size into N for easy reading in the window formulae
    N = array_size;

    // iterate through the array
    for( index = 0 ; index < N ; ++index )
    {
        // assign index to n instead of typecasting index to float for easier reading in formulae
        n = index;

        // assign w_float element address to value for easier reading in formulae
        value = &( array_data[ index ].w_float );

        // calculate window values based on window_id
        switch( object->window_id )
        {
            // rectangle
            //------------------------------------------------------------------
            case RECTANGLE          :   *value = 1;
                                        break;

            // hann
            //------------------------------------------------------------------
            case HANN               :   *value = 0.5 * ( 1 - Cosine( ( C_2_PI * n ) / ( N - 1 ) ) );
                                        break;

            // hamming
            //------------------------------------------------------------------
            case HAMMING            :   *value = 0.54 - 0.46 * Cosine( ( C_2_PI * n ) / ( N - 1 ) );
                                        break;

            // tukey
            //------------------------------------------------------------------
            case TUKEY              :   a = object->tukey_coeff;

                                        if( ( n >= 0 ) && ( n <= ( ( a * ( N - 1 ) ) / 2 ) ) )
                                        {
                                            *value = 0.5 * ( 1 + Cosine( C_PI * ( ( ( 2 * n ) / ( a * ( N - 1 ) ) ) - 1 ) ) );
                                        }
                                        else if( ( n >= ( ( a * ( N - 1 ) ) / 2 ) ) && ( n <= ( ( N - 1 ) * ( 1 - ( a / 2 ) ) ) ) )
                                        {
                                            *value = 1;
                                        }
                                        else if( ( n >= ( ( N - 1 ) * ( 1 - ( a / 2 ) ) ) ) && ( n <= ( N - 1 ) ) )
                                        {
                                            *value = 0.5 * ( 1 + Cosine( C_PI * ( ( ( 2 * n ) / ( a * ( N - 1 ) ) ) - ( 2 / a ) + 1 ) ) );
                                        }

                                        break;

            // cosine
            //------------------------------------------------------------------
            case COSINE             :   *value = Cosine( ( ( C_PI * n ) / ( N - 1 ) ) - C_PI_OVER_2 );
                                        break;

            // lanczos
            //------------------------------------------------------------------
            case LANCZOS            :   temp   = ( ( 2 * n ) / ( N - 1 ) ) - 1;
                                        temp   = ( temp == 0 ? C_FLOAT_MIN : temp );
                                        *value = NormalizedSinc( temp );
                                        break;

            // triangle
            //------------------------------------------------------------------
            case TRIANGLE           :   *value = ( 2 / ( N - 1 ) ) * ( ( ( N - 1 ) / 2 ) - Absolute( n - ( ( N - 1 ) / 2 ) ) );
                                        break;

            // gaussian
            //------------------------------------------------------------------
            case GAUSSIAN           :   a      = object->gaussian_coeff;
                                        *value = Power( C_E, ( -0.5 * Power( ( ( n - ( ( N - 1 ) / 2 ) ) / ( a * ( ( N - 1 ) / 2 ) ) ), 2 ) ) );
                                        break;

            // bartlett-hann
            //------------------------------------------------------------------
            case BARTLETT_HANN      :   *value = 0.62 - 0.48 * Absolute( n / ( N - 1 ) - 0.5 ) - 0.38 * Cosine( ( C_2_PI * n ) / ( N - 1 ) );
                                        break;

            //  blackman
            //------------------------------------------------------------------
            case BLACKMAN           :   a0     = ( 1 - object->blackman_coeff ) * 0.5;
                                        a1     = 0.5;
                                        a2     = object->blackman_coeff * 0.5;
                                        *value = a0 - a1 * Cosine( ( C_2_PI * n ) / ( N - 1 ) ) + a2 * Cosine( ( C_4_PI * n ) / ( N - 1 ) );
                                        break;

            // kaiser
            //------------------------------------------------------------------
            case KAISER             :   a      = object->kaiser_coeff;
                                        *value = BesselI0( C_PI * a * SquareRoot( 1 - Power( ( ( 2 * n ) / ( N - 1 ) - 1 ), 2 ) ) ) / BesselI0( C_PI * a );
                                        break;

            // nuttall
            //------------------------------------------------------------------
            case NUTTALL            :   a0     = 0.355768;
                                        a1     = 0.487396;
                                        a2     = 0.144232;
                                        a3     = 0.012604;
                                        *value = a0 - a1 * Cosine( ( C_2_PI * n ) / ( N - 1 ) ) + a2 * Cosine( ( C_4_PI * n ) / ( N - 1 ) ) - a3 * Cosine( ( C_6_PI * n ) / ( N - 1 ) );
                                        break;

            // blackman-harris
            //------------------------------------------------------------------
            case BLACKMAN_HARRIS    :   a0     = 0.35875;
                                        a1     = 0.48829;
                                        a2     = 0.14128;
                                        a3     = 0.01168;
                                        *value = a0 - a1 * Cosine( ( C_2_PI * n ) / ( N - 1 ) ) + a2 * Cosine( ( C_4_PI * n ) / ( N - 1 ) ) - a3 * Cosine( ( C_6_PI * n ) / ( N - 1 ) );
                                        break;

            // blackman-nuttall
            //------------------------------------------------------------------
            case BLACKMAN_NUTTALL   :   a0     = 0.3635819;
                                        a1     = 0.4891775;
                                        a2     = 0.1365995;
                                        a3     = 0.0106411;
                                        *value = a0 - a1 * Cosine( ( C_2_PI * n ) / ( N - 1 ) ) + a2 * Cosine( ( C_4_PI * n ) / ( N - 1 ) ) - a3 * Cosine( ( C_6_PI * n ) / ( N - 1 ) );
                                        break;

            // poisson
            //------------------------------------------------------------------
            case POISSON            :   a      = object->poisson_coeff;
                                        *value = Power( C_E, -Absolute( n - ( N - 1 ) / 2) * ( 1 / ( ( N - 1 ) / ( 2 * a ) ) ) );
                                        break;

            // hann-poisson
            //------------------------------------------------------------------
            case HANN_POISSON       :   a      = object->hann_poisson_coeff;
                                        *value = ( 0.5 * ( 1 - Cosine( ( C_2_PI * n ) / ( N - 1 ) ) ) ) * Power( C_E, -Absolute( n - ( N - 1 ) / 2) * ( 1 / ( ( N - 1 ) / ( 2 * a ) ) ) );
                                        break;

            // end window calculation
            //------------------------------------------------------------------

        }
    }

    garray_redraw( array );
}


//------------------------------------------------------------------------------
// windower_messge_parse - evaluates messages and sets window calculation variables
//------------------------------------------------------------------------------
void windower_message_parse( t_windower* object, t_symbol* selector, t_int items, t_atom* list )
{
    // get selector string
    const char* message = selector->s_name;

    // set ( accepts one argument specifying array name )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "set" ) )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            // get array name string from list item 0
            object->array_name = list[ 0 ].a_w.w_symbol;

            // notify of extra arguments
            if( items > 1 )
            {
                pd_error( object, "windower: set: extra arguments ignored" );
            }
        }
        else
        {
            pd_error( object, "windower: set: invalid argument type" );
        }

        // skip windower_bang() at the end of this function
        return;
    }

    // rectangle
    //--------------------------------------------------------------------------
    if( StringMatch( message, "rectangle" ) )
    {
        object->window_id = RECTANGLE;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: rectangle: extra arguments ignored" );
        }
    }

    // hann
    //--------------------------------------------------------------------------
    if( StringMatch( message, "hann" ) )
    {
        object->window_id = HANN;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: hann: extra arguments ignored" );
        }

    }

    // hanning ( hann )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "hanning" ) )
    {
        object->window_id = HANN;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: hanning: extra arguments ignored" );
        }

    }

    // hamming
    //--------------------------------------------------------------------------
    if( StringMatch( message, "hamming" ) )
    {
        object->window_id = HAMMING;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: hamming: extra arguments ignored" );
        }
    }

    // tukey ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "tukey" ) )
    {
        object->window_id = TUKEY;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->tukey_coeff = 0.5;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->tukey_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 0 and 1
            object->tukey_coeff = Clip( object->tukey_coeff, C_FLOAT_MIN, 1 );
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: tukey: extra arguments ignored" );
        }
    }

    // cosine
    //--------------------------------------------------------------------------
    if( StringMatch( message, "cosine" ) )
    {
        object->window_id = COSINE;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: cosine: extra arguments ignored" );
        }
    }

    // sine ( cosine )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "sine" ) )
    {
        object->window_id = COSINE;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: sine: extra arguments ignored" );
        }
    }

    // lanczos
    //--------------------------------------------------------------------------
    if( StringMatch( message, "lanczos" ) )
    {
        object->window_id = LANCZOS;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: lanczos: extra arguments ignored" );
        }
    }

    // triangle
    //--------------------------------------------------------------------------
    if( StringMatch( message, "triangle" ) )
    {
        object->window_id = TRIANGLE;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: triangle: extra arguments ignored" );
        }
    }

    // bartlett ( triangle )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "bartlett" ) )
    {
        object->window_id = TRIANGLE;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: bartlett: extra arguments ignored" );
        }
    }

    // gaussian ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "gaussian" ) )
    {
        object->window_id = GAUSSIAN;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->gaussian_coeff = 0.25;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->gaussian_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 0.0...01 and 1
            object->gaussian_coeff = Clip( object->gaussian_coeff, C_FLOAT_MIN, 0.5 );
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: gaussian: extra arguments ignored" );
        }
    }

    // bartlett-hann
    //--------------------------------------------------------------------------
    if( StringMatch( message, "bartlett-hann" ) )
    {
        object->window_id = BARTLETT_HANN;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: bartlett-hann: extra arguments ignored" );
        }
    }

    // blackman ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "blackman" ) )
    {
        object->window_id = BLACKMAN;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->blackman_coeff = 0.16;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->blackman_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 0 and 1
            object->blackman_coeff = Clip( object->blackman_coeff, 0, 0.25 );
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: blackman: extra arguments ignored" );
        }
    }

    // kaiser ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "kaiser" ) )
    {
        object->window_id = KAISER;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->kaiser_coeff = 3;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->kaiser_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 2/3 and 4.75
            object->kaiser_coeff = Clip( object->kaiser_coeff, 0.666666, 4.75);
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: kaiser: extra arguments ignored" );
        }
    }

    // nuttall
    //--------------------------------------------------------------------------
    if( StringMatch( message, "nuttall" ) )
    {
        object->window_id = NUTTALL;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: nuttall: extra arguments ignored" );
        }
    }

    // blackman-harris
    //--------------------------------------------------------------------------
    if( StringMatch( message, "blackman-harris" ) )
    {
        object->window_id = BLACKMAN_HARRIS;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: blackman-harris: extra arguments ignored" );
        }
    }

    // blackman-nuttall
    //--------------------------------------------------------------------------
    if( StringMatch( message, "blackman-nuttall" ) )
    {
        object->window_id = BLACKMAN_NUTTALL;

        // notify of extra arguments
        if( items > 0 )
        {
            pd_error( object, "windower: blackman-nuttall: extra arguments ignored" );
        }
    }

    // poisson ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "poisson" ) )
    {
        object->window_id = POISSON;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->poisson_coeff = 1;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->poisson_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 0 and max float
            object->poisson_coeff = Clip( object->poisson_coeff, 0, C_FLOAT_MAX);
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: poisson: extra arguments ignored" );
        }
    }

    // hann-poisson ( accepts one argument specifying window shape )
    //--------------------------------------------------------------------------
    if( StringMatch( message, "hann-poisson" ) )
    {
        object->window_id = HANN_POISSON;

        // set coefficient
        if( items == 0 )
        {
            // default value
            object->hann_poisson_coeff = 1;
        }
        else if( items > 0 )
        {
            // get float value from list item 0
            object->hann_poisson_coeff = list[ 0 ].a_w.w_float;

            // constrain value between 0 and max float
            object->hann_poisson_coeff = Clip( object->hann_poisson_coeff, 0, C_FLOAT_MAX);
        }

        // notify of extra arguments
        if( items > 1 )
        {
            pd_error( object, "windower: hann-poisson: extra arguments ignored" );
        }
    }

    // end message parsing
    //--------------------------------------------------------------------------

    windower_bang( object );
}


//------------------------------------------------------------------------------
// windower_bang - causes status windower output
//------------------------------------------------------------------------------
void windower_bang( t_windower* object )
{
    windower_fill_array( object );
}


//------------------------------------------------------------------------------
// windower_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
static void* windower_new( t_symbol* selector, t_int items, t_atom* list )
{
    // declare a pointer to this class
    t_windower* object;

    // generate a new object and save its pointer in "object"
    object = ( t_windower* )pd_new( windower_class );

    t_symbol init_selector;
    t_atom init_list;

    // parse initialization arguments
    //--------------------------------------------------------------------------

    // array name
    if( items > 0 )
    {
        // make a "selector" symbol from 'set'
        init_selector.s_name = "set";

        // make a list of arguments from list argument 0's symbol
        init_list.a_w.w_symbol = list[ 0 ].a_w.w_symbol;
        init_list.a_type = list[ 0 ].a_type;

        // initialize with constructed messages
        windower_message_parse( object, &init_selector, 1, &init_list );
    }

    // window + argument
    if( items > 2 )
    {
        if( ( list[ 1 ].a_type == A_SYMBOL ) && ( list[ 2 ].a_type == A_FLOAT ) )
        {
            // make a "selector" symbol from list argument 1's name
            init_selector.s_name = list[ 1 ].a_w.w_symbol->s_name;

            // make a list of arguments from list argument 2's float
            init_list.a_w.w_float = list[ 2 ].a_w.w_float;

            // initialize with constructed messages
            windower_message_parse( object, &init_selector, 1, &init_list );
        }
        else if( list[ 1 ].a_type == A_SYMBOL )
        {
            // make a "selector" symbol from list argument 1's name
            init_selector.s_name = list[ 1 ].a_w.w_symbol->s_name;

            // initialize with constructed messages
            windower_message_parse( object, &init_selector, 0, &init_list );
        }
        else
        {
            // notify of invalid arguments
            pd_error( object, "windower: initalization: invalid arguments" );
        }
    }
    else if( items > 1 )
    {
        if( list[ 1 ].a_type == A_SYMBOL )
        {
            // make a "selector symbol from list argument 1's name
            init_selector.s_name = list[ 1 ].a_w.w_symbol->s_name;

            // initialize with constructed messages
            windower_message_parse( object, &init_selector, 0, &init_list );
        }
        else
        {
            // notify of invalid arguments
            pd_error( object, "windower: initalization: invalid arguments" );
        }
    }


    if( items > 3 )
    {
        // notify of extra arguments
        pd_error( object, "windower: initalization: extra arguments ignored" );
    }

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// windower setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void windower_setup( void )
{
    // create a new class and assign its pointer to windower_class
    windower_class = class_new( gensym( "windower" ), ( t_newmethod )windower_new, 0, sizeof( t_windower ), 0, A_GIMME, 0 );

    // add message handlers
    class_addmethod( windower_class, ( t_method )windower_message_parse, gensym( "anything" ), A_GIMME, 0 );

    // add bang handler
    class_addbang( windower_class, ( t_method )windower_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
