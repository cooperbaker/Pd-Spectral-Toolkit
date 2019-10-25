//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  utility.inline.h
//
//  Utility inline function definitions
//
//  Created by Cooper Baker on 4/15/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// headers
//------------------------------------------------------------------------------

// system math library
#include <math.h>

// system string library
#include <string.h>

// system standard library
#include <stdlib.h>

// system standard io library
#include <stdio.h>


//------------------------------------------------------------------------------
// Absolute - returns the absolute value of number
//------------------------------------------------------------------------------
inline t_float Absolute( t_float number )
{
    return fabsf( number );
}


//------------------------------------------------------------------------------
// Power - returns the value of base raised to exponent
//------------------------------------------------------------------------------
inline t_float Power( t_float base, t_float exponent )
{
    return powf( base, exponent );
}


//------------------------------------------------------------------------------
// Cosine - returns the cosine of number
//------------------------------------------------------------------------------
inline t_float Cosine( t_float number )
{
    return cosf( number );
}


//------------------------------------------------------------------------------
// ArcCosine - returns the inverse cosine of number
//------------------------------------------------------------------------------
inline t_float ArcCosine( t_float number )
{
    return acosf( number );
}


//------------------------------------------------------------------------------
// HyperbolicCosine - returns the hyperbolic cosine of number
//------------------------------------------------------------------------------
inline t_float HyperbolicCosine( t_float number )
{
    return coshf( number );
}


//------------------------------------------------------------------------------
// ArcHyperbolicCosine - returns the inverse hyperbolic cosine of number
//------------------------------------------------------------------------------
inline t_float ArcHyperbolicCosine( t_float number )
{
    return acoshf( number );
}


//------------------------------------------------------------------------------
// Sine - returns the sine of number
//------------------------------------------------------------------------------
inline t_float Sine( t_float number )
{
    return sinf( number );
}


//------------------------------------------------------------------------------
// NormalizedSinc - returns the normalized sinc of number
//------------------------------------------------------------------------------
inline t_float NormalizedSinc( t_float number )
{
    return ( Sine( C_PI * number ) / ( C_PI * number ) );
}


//------------------------------------------------------------------------------
// ArcTangent - returns the arc tangent of number
//------------------------------------------------------------------------------
inline t_float ArcTangent( t_float number )
{
    return atanf( number );
}


//------------------------------------------------------------------------------
// ArcTangent2 - returns the arc tangent 2 of a complex number
//------------------------------------------------------------------------------
inline t_float ArcTangent2( t_float imaginary, t_float real )
{
    return atan2f( imaginary, real );
}

//------------------------------------------------------------------------------
// SquareRoot - returns the square root of number
//------------------------------------------------------------------------------
inline t_float SquareRoot( t_float number )
{
    return sqrtf( number );
}


//------------------------------------------------------------------------------
// Modulo - returns the remainder of numerator divided by denominator
//------------------------------------------------------------------------------
inline t_float Modulo( t_float numerator, t_float denominator )
{
    return fmodf( numerator, denominator );
}


//------------------------------------------------------------------------------
// FixNan - returns zero if number is not a number
//------------------------------------------------------------------------------
inline t_float FixNan( t_float number )
{
    return ( isnan( number ) ? 0.0f : number );
}


//------------------------------------------------------------------------------
// FixInf - returns zero if number is infinity
//------------------------------------------------------------------------------
inline t_float FixInf( t_float number )
{
    return isinf( number ) ? ( number > 0 ? FLT_MAX : -FLT_MAX ) : number;
}


//------------------------------------------------------------------------------
// FixNanInf - returns zero if number is not a number or infinity
//------------------------------------------------------------------------------
inline t_float FixNanInf( t_float number )
{
    return ( isinf( number ) ? ( number > 0 ? FLT_MAX : -FLT_MAX ) : ( isnan( number ) ? 0.0f : number ) );
}


//------------------------------------------------------------------------------
// DegToRad - converts degrees to radians
//------------------------------------------------------------------------------
inline t_float DegToRad( t_float degrees )
{
    return ( degrees * C_PI_OVER_180 );
}


//------------------------------------------------------------------------------
// DegToTurn - converts degrees to turns
//------------------------------------------------------------------------------
inline t_float DegToTurn( t_float degrees )
{
    return ( degrees * C_1_OVER_360 );
}


//------------------------------------------------------------------------------
// RadToDeg - converts radians to degrees
//------------------------------------------------------------------------------
inline t_float RadToDeg( t_float radians )
{
    return ( radians * C_180_OVER_PI );
}


//------------------------------------------------------------------------------
// RadToTurn - converts radians to turns
//------------------------------------------------------------------------------
inline t_float RadToTurn( t_float radians )
{
    return ( radians * C_1_OVER_2_PI );
}


//------------------------------------------------------------------------------
// TurnToDeg - convers turns to degrees
//------------------------------------------------------------------------------
inline t_float TurnToDeg( t_float turns )
{
    return ( turns * 360.0 );
}


//------------------------------------------------------------------------------
// TurnToRad - convers turns to radians
//------------------------------------------------------------------------------
inline t_float TurnToRad( t_float turns )
{
    return ( turns * C_2_PI );
}


//------------------------------------------------------------------------------
// Complex - returns a t_complex from two t_floats
//------------------------------------------------------------------------------
inline t_complex Complex( t_float real, t_float imaginary )
{
    t_complex c;

    c.r = real;
    c.i = imaginary;

    return c;
}


//------------------------------------------------------------------------------
// ComplexAdd - complex addition
//------------------------------------------------------------------------------
inline t_complex ComplexAdd( t_complex a, t_complex b )
{
    t_complex c;

    c.r = a.r + b.r;
    c.i = a.i + b.i;

    return c;
}


//------------------------------------------------------------------------------
// ComplexSubtract - complex subtraction
//------------------------------------------------------------------------------
inline t_complex ComplexSubtract( t_complex a, t_complex b )
{
    t_complex c;

    c.r = a.r - b.r;
    c.i = a.i - b.i;

    return c;
}


//------------------------------------------------------------------------------
// Complex Multiply - complex multiplication
//------------------------------------------------------------------------------
inline t_complex ComplexMultiply( t_complex a, t_complex b )
{
    t_complex c;

    c.r = a.r * b.r - a.i * b.i;
    c.i = a.i * b.r + a.r * b.i;

    return c;
}


//------------------------------------------------------------------------------
// ComplexDivide - complex division
//------------------------------------------------------------------------------
inline t_complex ComplexDivide( t_complex a, t_complex b )
{
    t_complex c;

    t_float r;
    t_float den;

    if( fabs( b.r ) >= fabs( b.i ) )
    {
        r   =   b.i / b.r;
        den =   b.r + r * b.i;
        c.r = ( a.r + r * a.i ) / den;
        c.i = ( a.i - r * a.r ) / den;
    }
    else
    {
        r   =   b.r / b.i;
        den =   b.i + r * b.r;
        c.r = ( a.r * r + a.i ) / den;
        c.i = ( a.i * r - a.r ) / den;
    }

    return c;
}


//------------------------------------------------------------------------------
// ComplexSquareRoot - complex square root
//------------------------------------------------------------------------------
inline t_complex ComplexSquareRoot( t_complex z )
{
    t_complex c;

    t_float x;
    t_float y;
    t_float w;
    t_float r;

    if( ( z.r == 0.0 ) && ( z.i == 0.0 ) )
    {
        c.r = 0.0;
        c.i = 0.0;

        return c;
    }
    else
    {
        x = fabs( z.r );
        y = fabs( z.i );
        if( x >= y )
        {
            r = y / x;
            w = sqrt( x ) * sqrt( 0.5 * ( 1.0 + sqrt( 1.0 + r * r ) ) );
        }
        else
        {
            r = x / y;
            w = sqrt( y ) * sqrt( 0.5 * ( r + sqrt( 1.0 + r * r ) ) );
        }
        if( z.r >= 0.0 )
        {
            c.r = w;
            c.i = z.i / ( 2.0 * w );
        }
        else
        {
            c.i = ( z.i >= 0 ) ? w : -w;
            c.r = z.i / ( 2.0 * c.i );
        }

        return c;
    }
}


//------------------------------------------------------------------------------
// RealComplexMultiply - real number multiplied by complex number
//------------------------------------------------------------------------------
inline t_complex RealComplexMultiply( t_float real, t_complex a )
{
    t_complex c;

    c.r = real * a.r;
    c.i = real * a.i;

    return c;
}


//------------------------------------------------------------------------------
// ComplexAbsolute - absolute value of complex number
//------------------------------------------------------------------------------
inline t_float ComplexAbsolute( t_complex z )
{
    t_float x;
    t_float y;
    t_float ans;
    t_float temp;

    x = fabs( z.r );
    y = fabs( z.i );

    if( x == 0.0 )
    {
        ans = y;
    }
    else if (y == 0.0 )
    {
        ans = x;
    }
    else if( x > y )
    {
        temp = y / x;
        ans = x * sqrt( 1.0 + temp * temp );
    }
    else
    {
        temp = x / y;
        ans  = y * sqrt( 1.0 + temp * temp );
    }

    return ans;
}


//------------------------------------------------------------------------------
// StringMatch - returns 1 if strings match, 0 if not
//------------------------------------------------------------------------------
inline t_int StringMatch( const char* string_a, const char* string_b )
{
    return ( ( strcmp( string_a, string_b ) == 0 ) ? 1 : 0 );
}


//------------------------------------------------------------------------------
// Clip - constrains a number between bottom and top
//------------------------------------------------------------------------------
inline t_float Clip( t_float number, t_float bottom, t_float top )
{
    return number < bottom ? bottom : ( number > top ? top : number );
}


//------------------------------------------------------------------------------
// ClipMin - constrains minimum value
//------------------------------------------------------------------------------
inline t_float ClipMin( t_float number, t_float minimum )
{
    return number < minimum ? minimum : number;
}


//------------------------------------------------------------------------------
// ClipMax - constrains maximum value
//------------------------------------------------------------------------------
inline t_float ClipMax( t_float number, t_float maximum )
{
    return number > maximum ? maximum : number;
}


//------------------------------------------------------------------------------
// Round - rounds a number to the nearest integer
//------------------------------------------------------------------------------
inline t_float Round( t_float number )
{
    return ( number >= 0 ) ? ( int )( number + 0.5 ) : ( int )( number - 0.5 );
}


//------------------------------------------------------------------------------
// Polynomial - evaluates a polynomial
//------------------------------------------------------------------------------
inline t_float Polynomial( const t_float *coeff, const t_int n, const t_float x )
{
    t_int i;

    t_float result = coeff[ n ];

    for( i = n - 1 ; i >= 0 ; i-- )
    {
        result *= x + coeff[ i ];
    }

    return result;
}


//------------------------------------------------------------------------------
// BesselI0 - zeroth order modified bessel function of the first kind
//------------------------------------------------------------------------------
inline t_float BesselI0( t_float number )
{
    t_float ax;
    t_float y;
    t_float z;
    t_float i0p [ 14 ];
    t_float i0q [ 5  ];
    t_float i0pp[ 5  ];
    t_float i0qq[ 6  ];

    ax = fabsf( number );

    if( ax < 15.0 )
    {
        y = number * number;
        return ( Polynomial( i0p, 13, y ) / Polynomial( i0q, 4, 225.0f - y ) );
    }
    else
    {
        z = 1.0 - 15.0 / ax;
        return ( expf( ax ) * Polynomial( i0pp, 4, z ) / ( Polynomial( i0qq, 5, z ) * sqrtf( ax ) ) );
    }
}


//------------------------------------------------------------------------------
// AToDb - Converts amplitude to decibel values
//------------------------------------------------------------------------------
inline t_float AToDb( t_float amplitude )
{
    return ( 20.0 * log10( amplitude ) );
}


//------------------------------------------------------------------------------
// DbToA - Converts decibel to amplitude values
//------------------------------------------------------------------------------
inline t_float DbToA( t_float decibels )
{
    return ( pow( 10.0, decibels / 20.0 ) );
}


//------------------------------------------------------------------------------
// WrapPosNegPi - wraps a number between -pi and pi
//------------------------------------------------------------------------------
inline t_float WrapPosNegPi( t_float number )
{
    return ( number > 0 ) ? ( fmodf( number + C_PI, C_2_PI ) - C_PI ) : ( fmodf( number - C_PI, C_2_PI ) + C_PI );
}


//------------------------------------------------------------------------------
// RotateArray - rotates the values in an array
//------------------------------------------------------------------------------
inline void RotateArray( t_float* array, t_float* temp_array, t_int shift, t_int size )
{
    // wrap negative shift value into correct range
    while( shift < -size )
    {
        shift += size;
    }

    // wrap positive shift value into correct range
    while( shift > size )
    {
        shift -= size;
    }

    // make all shifts positive
    if( shift < 0 )
    {
        shift += size;
    }

    // do nothing if no shift is necessary
    if( ( shift == 0 ) || ( shift == size ) )
    {
        return;
    }

    // rotate array into temp array by copying offset parts
    memcpy( temp_array + shift, array, ( size - shift ) * sizeof( t_float ) );
    memcpy( temp_array, array + ( size - shift ), shift * sizeof( t_float ) );

    // copy temp array back into array
    memcpy( array, temp_array, size * sizeof( t_float ) );
}


//------------------------------------------------------------------------------
// MayerRealFFTUnpack - unpacks mayer real fft data into real and imag arrays
//------------------------------------------------------------------------------
inline void MayerRealFFTUnpack( t_float* rfft_data, t_float* real_data, t_float* imag_data, t_int frames )
{
    t_int half_frames = frames * 0.5;

    // copy real portion of rfft_data into real_data array
    memcpy( real_data, rfft_data, ( half_frames + 1 ) * sizeof( t_float ) );

    // set imag_data pointer location for decrementing in while loop
    imag_data += half_frames;

    // move rfft_data pointer to beginning of imaginary data for incrementing in while loop
    rfft_data += half_frames;

    // flip the imaginary data from rfft_data into imag_data array
    while( --half_frames )
    {
        *( --imag_data ) = - *( ++rfft_data );
    }

    // Mayer real fft data packing scheme as described by Miller Puckette:
    //--------------------------------------------------------------------------
    // It's weird and I don't know why it's done this way... there are N output
    // points to a 'real-to-complex' FFT: N/2+1 real points from DC to Nyquist
    // inclusive, and N/2-1 points for the imaginary parts because the DC and
    // Nyquist channels are real-valued only.  These imaginary points are stored
    // backward, from N/2-1 to 1, in locations N/2+1 to N-1 .  I don't remember
    // them being negated but maybe that's so too.
}


//------------------------------------------------------------------------------
// MayerRealIFFTPack - Packs real and imag data into rifft array for Mayer rifft
//------------------------------------------------------------------------------
inline void MayerRealIFFTPack( t_float* rifft_data, t_float* real_data, t_float* imag_data, t_int frames )
{
    t_int half_frames = frames * 0.5;

    // copy real_data array into rifft_data array
    memcpy( rifft_data, real_data, ( half_frames + 1 ) * sizeof( t_float ) );

    // set imag_data pointer location for incrementing in while loop
    imag_data += half_frames;

    // move rifft_data pointer to end of array for decrementing in while loop
    rifft_data += half_frames;

    // flip the imaginary data from imag_data array into rifft_data array
    while( --half_frames )
    {
        *( ++rifft_data ) = - *( --imag_data );
    }
}


//------------------------------------------------------------------------------
// BubbleSort - sorts array into ascending order
//------------------------------------------------------------------------------
inline void BubbleSort( t_float array[], t_int size )
{
    int     passes;
    int     sorted;
    int     index;
    t_float temp;

    for( passes = size - 1.0 ; passes > 0 ; --passes )
    {
        sorted = TRUE;

        for( index = 0 ; index < passes ; ++index )
        {
            if( array[ index ] > array[ index + 1 ] )
            {
                temp               = array[ index     ];
                array[ index     ] = array[ index + 1 ];
                array[ index + 1 ] = temp;

                sorted = FALSE;
            }
        }

        if( sorted )
        {
            return;
        }
    }
}


//------------------------------------------------------------------------------
// QuickSort - sorts array into ascending order
//------------------------------------------------------------------------------
inline void QuickSort( t_float* array, t_int beginning, t_int end )
{
    if( end > ( beginning + 1 ) )
    {
        t_float pivot = array[ beginning ];
        t_int   left  = beginning + 1;
        t_int   right = end;
        t_float temp;

        while( left < right )
        {
            if( array[ left ] <= pivot )
            {
                ++left;
            }
            else
            {
                --right;

                temp           = array[ left  ];
                array[ left  ] = array[ right ];
                array[ right ] = temp;
            }
        }

        --left;

        temp               = array[ left      ];
        array[ left      ] = array[ beginning ];
        array[ beginning ] = temp;

        QuickSort( array, beginning, left );
        QuickSort( array, right,     end  );
    }
}


//------------------------------------------------------------------------------
// Lerp - linear interpolation
//------------------------------------------------------------------------------
inline t_float Lerp( t_float value_a, t_float value_b, t_float fraction )
{
    return( value_a + fraction * ( value_b - value_a ) );
}


//------------------------------------------------------------------------------
// Reciprocal - returns the reciprocal of a number
//------------------------------------------------------------------------------
inline t_float Reciprocal( t_float number )
{
    return ( 1.0 / number );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
