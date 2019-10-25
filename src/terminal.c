//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  terminal.c
//
//  Accepts a command line shell instruction and outputs the result
//
//  Created by Cooper Baker on 3/29/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// m_pd.h - main header for Pd
//------------------------------------------------------------------------------
#include "m_pd.h"
#include "stdio.h"
#include "string.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"

#define STRING_MAX 65535


//------------------------------------------------------------------------------
// terminal_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* terminal_class;


//------------------------------------------------------------------------------
// terminal - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct terminal
{
    // this object - must always be first variable in struct
    t_object object;

    // pointer to the outlet
    t_outlet* outlet;

    // character string to hold a terminal command
    char* terminal_command[ 1024 ];

} t_terminal;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
char* terminal_itoa  ( t_int number );
void  terminal_parse ( t_terminal* object, t_symbol* selector, t_int items, t_atom* list );
void  terminal_bang  ( t_terminal* object );
void* terminal_new   ( void );
void  terminal_setup ( void );


//------------------------------------------------------------------------------
// terminal_itoa - converts integers to ascii strings
//------------------------------------------------------------------------------
char* terminal_itoa( t_int number )
{
    t_int i = 0;
    t_int j = 0;
    t_int sign = number;
    char character;
    static char string[ STRING_MAX ];

    // if number is negative
    if( sign < 0 )
    {
        // make the number positive
        number = -number;
    }

    // generate digits in reverse order
    for( number = number ; number > 0 ; number /= 10 )
    {
        // calculate and store ascii values
        string[ i ] = number % 10 + '0';
        ++i;
    }

    // if number is negative
    if( sign < 0 )
    {
        // add the minus sign
        string[ i++ ] = '-';
    }

    // null terminate the string
    string[ i ] = '\0';

    // reverse the string
    for( i = 0, j = strlen( string ) - 1 ; i < j ; ++i, --j )
    {
        character   = string[ i ];
        string[ i ] = string[ j ];
        string[ j ] = character;
    }

    // hand the string to the calling function
    return string;
}


//------------------------------------------------------------------------------
// terminal_bang - causes status list output
//------------------------------------------------------------------------------
void terminal_bang( t_terminal* object )
{
    // location of null terminator in each output string
    t_int null_terminator;

    // an os FILE pointer for the pipe
    FILE* terminal_pipe;

    // string to hold what comes out of the pipe
    char output[ STRING_MAX ];

    // open a "read" pipe and send it the object's command
    terminal_pipe = popen( ( char* )object->terminal_command, "r" );

    // move through the output to its end
    while( fgets( output, STRING_MAX, terminal_pipe ) != NULL )
    {
        // find the null terminator location
        null_terminator = strchr( output, '\n' ) - output;

        // remove the null terminator
        output[ null_terminator ] = 0;

        // send the output out the object's outlet
        outlet_anything( object->outlet, gensym( output ), 0, NULL );
    }

    // close the pipe
    pclose( terminal_pipe );
}


//------------------------------------------------------------------------------
// terminal_parse - parses list input
//------------------------------------------------------------------------------
void terminal_parse( t_terminal* object, t_symbol* selector, t_int items, t_atom* list )
{
    // index to keep track of position in input list
    t_int item_index = 0;

    // clear out the last command
    memset( object->terminal_command, 0, 1024 * sizeof( char ) );

    // copy the first word into the command string
    strcpy( ( char* )object->terminal_command, selector->s_name );

    // iterate through the input mesage list
    for( item_index = 0 ; item_index < items ; ++item_index )
    {
        // if the list item is a symbol
        if( list[ item_index ].a_type == A_SYMBOL )
        {
            // first add a space then copy the item into the command string
            strcat( ( char* )object->terminal_command, " " );
            strcat( ( char* )object->terminal_command, list[ item_index ].a_w.w_symbol->s_name );
        }
        // if the list item is a float
        else if( list[ item_index ].a_type == A_FLOAT )
        {
            // first add a space then copy the item into the command string
            strcat( ( char* )object->terminal_command, " " );
            strcat( ( char* )object->terminal_command, terminal_itoa( list[ item_index ].a_w.w_float ) );
        }
    }

    // call bang to trigger output
    terminal_bang( object );
}


//------------------------------------------------------------------------------
// terminal_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* terminal_new( void )
{
    // declare a pointer to this class
    t_terminal* object;

    // generate a new object and save its pointer in "object"
    object = ( t_terminal* )pd_new( terminal_class );

    // create an outlet for anything
    object->outlet = outlet_new( &object->object, gensym( "anything" ) );

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// terminal setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void terminal_setup( void )
{
    // create a new class and assign its pointer to terminal_class
    terminal_class = class_new( gensym( "terminal" ), ( t_newmethod )terminal_new, 0, sizeof( t_terminal ), 0, 0 );

    // add message handler, responding to any message
    class_addmethod( terminal_class, ( t_method )terminal_parse, gensym( "anything" ), A_GIMME, 0 );

    // add bang handler
    class_addbang( terminal_class, ( t_method )terminal_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
