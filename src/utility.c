//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  utility.c
//
//  Utility function definitions
//
//  Created by Cooper Baker on 4/15/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// headers
//------------------------------------------------------------------------------
#include "utility.h"


//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------
extern t_float          Absolute                ( t_float number );
extern t_float          Power                   ( t_float base, t_float exponent );
extern t_float          Cosine                  ( t_float number );
extern t_float          ArcCosine               ( t_float number );
extern t_float          HyperbolicCosine        ( t_float number );
extern t_float          ArcHyperbolicCosine     ( t_float number );
extern t_float          Sine                    ( t_float number );
extern t_float          NormalizedSinc          ( t_float number );
extern t_float          ArcTangent              ( t_float number );
extern t_float          ArcTangent2             ( t_float imaginary, t_float real );
extern t_float          SquareRoot              ( t_float number );
extern t_float          Modulo                  ( t_float numerator, t_float denominator );
extern t_float          FixNan                  ( t_float number );
extern t_float          FixInf                  ( t_float number );
extern t_float          FixNanInf               ( t_float number );
extern t_float          DegToRad                ( t_float degrees );
extern t_float          DegToTurn               ( t_float degrees );
extern t_float          RadToDeg                ( t_float radians );
extern t_float          RadToTurn               ( t_float radians );
extern t_float          TurnToDeg               ( t_float turns );
extern t_float          TurnToRad               ( t_float turns );
extern t_complex        Complex                 ( t_float real, t_float imaginary );
extern t_complex        ComplexAdd              ( t_complex a, t_complex b );
extern t_complex        ComplexSubtract         ( t_complex a, t_complex b );
extern t_complex        ComplexMultiply         ( t_complex a, t_complex b );
extern t_complex        ComplexDivide           ( t_complex a, t_complex b );
extern t_complex        ComplexSquareRoot       ( t_complex z );
extern t_complex        RealComplexMultiply     ( t_float real, t_complex a );
extern t_float          ComplexAbsolute         ( t_complex z );
extern t_int            StringMatch             ( const char* string_a, const char* string_b );
extern t_float          Clip                    ( t_float number, t_float bottom, t_float top );
extern t_float          ClipMin                 ( t_float number, t_float minimum );
extern t_float          ClipMax                 ( t_float number, t_float maximum );
extern t_float          Round                   ( t_float number );
extern t_float          Polynomial              ( const t_float *coeff, const t_int n, const t_float x );
extern t_float          BesselI0                ( t_float number );
extern t_float          AToDb                   ( t_float amplitude );
extern t_float          DbToA                   ( t_float decibels );
extern t_float          WrapPosNegPi            ( t_float number );
extern void             RotateArray             ( t_float* array, t_float* temp_array, t_int shift, t_int size );
extern void             MayerRealFFTUnpack      ( t_float* rfft_data, t_float* real_data, t_float* imag_data, t_int frames );
extern void             MayerRealIFFTPack       ( t_float* rifft_data, t_float* real_data, t_float* imag_data, t_int frames );
extern void             BubbleSort              ( t_float array[], t_int size );
extern void             QuickSort               ( t_float* array, t_int beginning, t_int end );
extern t_float          Lerp                    ( t_float value_a, t_float value_b, t_float fraction );
extern t_float          Reciprocal              ( t_float number );


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
