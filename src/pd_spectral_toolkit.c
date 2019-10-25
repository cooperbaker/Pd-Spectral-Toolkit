//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  pd_spectral_toolkit.c
//
//  encapsulation object for pd spectral toolkit library
//
//  Created by Tom Erbe on 6/22/19
//  Copyright (c) 2019 Tom Erbe. All rights reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// include
//------------------------------------------------------------------------------
#include "m_pd.h"


//------------------------------------------------------------------------------
// pd_spectral_toolkit_class - pointer type of this object
//------------------------------------------------------------------------------
static t_class* pd_spectral_toolkit_class;


//------------------------------------------------------------------------------
// spectraltoolkit_new - creates a new spectraltoolkit object
//------------------------------------------------------------------------------
static void* pd_spectral_toolkit_new()
{
    // instantiate a spectraltoolkit object
    t_object* x = ( t_object* )pd_new( pd_spectral_toolkit_class );

    // return the object pointer
    return( x );
}


//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------
void setup_0x210x260x26_tilde ();
void setup_0x210x3d_tilde     ();
void setup_0x210x7c0x7c_tilde ();
void setup_0x21_tilde         ();
void setup_0x260x26_tilde     ();
void setup_0x25_tilde         ();
void setup_0x3c0x3d_tilde     ();
void setup_0x3c_tilde         ();
void setup_0x3d0x3d_tilde     ();
void setup_0x3e0x3d_tilde     ();
void setup_0x3e_tilde         ();
void setup_0x7c0x7c_tilde     ();
void amptodb_tilde_setup      ();
void amptomag_tilde_setup     ();
void binindex_tilde_setup     ();
void binmax_tilde_setup       ();
void binmin_tilde_setup       ();
void binmix_tilde_setup       ();
void binmonitor_tilde_setup   ();
void binsort_tilde_setup      ();
void bintrim_tilde_setup      ();
void bitsafe_tilde_setup      ();
void blocksmooth_tilde_setup  ();
void cartoamp_tilde_setup     ();
void cartodb_tilde_setup      ();
void cartofreq_tilde_setup    ();
void cartomag_tilde_setup     ();
void cartophase_tilde_setup   ();
void cartopolar_tilde_setup   ();
void cmplxabs_tilde_setup     ();
void cmplxadd_tilde_setup     ();
void cmplxdiv_tilde_setup     ();
void cmplxmult_tilde_setup    ();
void cmplxsqrt_tilde_setup    ();
void cmplxsub_tilde_setup     ();
void countwrap_setup          ();
void ctltosig_tilde_setup     ();
void dbtoamp_tilde_setup      ();
void dbtomag_tilde_setup      ();
void degtorad_tilde_setup     ();
void degtoturn_tilde_setup    ();
void dspbang_tilde_setup      ();
void freqsieve_tilde_setup    ();
void freqtocar_tilde_setup    ();
void freqtocar_tilde_setup    ();
void freqtophase_tilde_setup  ();
void freqtopolar_tilde_setup  ();
void fundfreq_tilde_setup     ();
void harmprod_tilde_setup     ();
void magtoamp_tilde_setup     ();
void magtodb_tilde_setup      ();
void magtrim_tilde_setup      ();
void monitor_tilde_setup      ();
void neg_tilde_setup          ();
void oscbank_tilde_setup      ();
void pafft_tilde_setup        ();
void paifft_tilde_setup       ();
void partconv_tilde_setup     ();
void peaks_tilde_setup        ();
void phaseaccum_tilde_setup   ();
void phasedelta_tilde_setup   ();
void phasetofreq_tilde_setup  ();
void piwrap_tilde_setup       ();
void polartocar_tilde_setup   ();
void polartofreq_tilde_setup  ();
void radtodeg_tilde_setup     ();
void radtoturn_tilde_setup    ();
void recip_tilde_setup        ();
void rgbtable_setup           ();
void rotate_tilde_setup       ();
void rounder_tilde_setup      ();
void scale_tilde_setup        ();
void sigtoctl_tilde_setup     ();
void softclip_tilde_setup     ();
void tabindex_tilde_setup     ();
void trunc_tilde_setup        ();
void turntodeg_tilde_setup    ();
void turntorad_tilde_setup    ();
void valleys_tilde_setup      ();
void windower_setup           ();
void winfft_tilde_setup       ();
void winifft_tilde_setup      ();

// posix only
#ifndef NT
    void terminal_setup();
#endif



//------------------------------------------------------------------------------
// spectraltoolkit_setup - setup routine for the objects
//------------------------------------------------------------------------------
void pd_spectral_toolkit_setup()
{
    // create Pd Spectral Toolkit class
    pd_spectral_toolkit_class = class_new( gensym( "pd_spectral_toolkit" ), pd_spectral_toolkit_new, 0, sizeof( t_object ), CLASS_NOINLET, 0 );

    // setup the objects
    setup_0x210x260x26_tilde ();
    setup_0x210x3d_tilde     ();
    setup_0x210x7c0x7c_tilde ();
    setup_0x21_tilde         ();
    setup_0x260x26_tilde     ();
    setup_0x25_tilde         ();
    setup_0x3c0x3d_tilde     ();
    setup_0x3c_tilde         ();
    setup_0x3d0x3d_tilde     ();
    setup_0x3e0x3d_tilde     ();
    setup_0x3e_tilde         ();
    setup_0x7c0x7c_tilde     ();
    amptodb_tilde_setup      ();
    amptomag_tilde_setup     ();
    binindex_tilde_setup     ();
    binmax_tilde_setup       ();
    binmin_tilde_setup       ();
    binmix_tilde_setup       ();
    binmonitor_tilde_setup   ();
    binsort_tilde_setup      ();
    bintrim_tilde_setup      ();
    bitsafe_tilde_setup      ();
    blocksmooth_tilde_setup  ();
    cartoamp_tilde_setup     ();
    cartodb_tilde_setup      ();
    cartofreq_tilde_setup    ();
    cartomag_tilde_setup     ();
    cartophase_tilde_setup   ();
    cartopolar_tilde_setup   ();
    cmplxabs_tilde_setup     ();
    cmplxadd_tilde_setup     ();
    cmplxdiv_tilde_setup     ();
    cmplxmult_tilde_setup    ();
    cmplxsqrt_tilde_setup    ();
    cmplxsub_tilde_setup     ();
    countwrap_setup          ();
    ctltosig_tilde_setup     ();
    dbtoamp_tilde_setup      ();
    dbtomag_tilde_setup      ();
    degtorad_tilde_setup     ();
    degtoturn_tilde_setup    ();
    dspbang_tilde_setup      ();
    freqsieve_tilde_setup    ();
    freqtocar_tilde_setup    ();
    freqtocar_tilde_setup    ();
    freqtophase_tilde_setup  ();
    freqtopolar_tilde_setup  ();
    fundfreq_tilde_setup     ();
    harmprod_tilde_setup     ();
    magtoamp_tilde_setup     ();
    magtodb_tilde_setup      ();
    magtrim_tilde_setup      ();
    monitor_tilde_setup      ();
    oscbank_tilde_setup      ();
    pafft_tilde_setup        ();
    paifft_tilde_setup       ();
    partconv_tilde_setup     ();
    peaks_tilde_setup        ();
    phaseaccum_tilde_setup   ();
    phasedelta_tilde_setup   ();
    phasetofreq_tilde_setup  ();
    piwrap_tilde_setup       ();
    polartocar_tilde_setup   ();
    polartofreq_tilde_setup  ();
    radtodeg_tilde_setup     ();
    radtoturn_tilde_setup    ();
    recip_tilde_setup        ();
    rgbtable_setup           ();
    rotate_tilde_setup       ();
    rounder_tilde_setup      ();
    scale_tilde_setup        ();
    sigtoctl_tilde_setup     ();
    softclip_tilde_setup     ();
    tabindex_tilde_setup     ();
    trunc_tilde_setup        ();
    turntodeg_tilde_setup    ();
    turntorad_tilde_setup    ();
    valleys_tilde_setup      ();
    windower_setup           ();
    winfft_tilde_setup       ();
    winifft_tilde_setup      ();

    // posix only
    #ifndef NT
        terminal_setup();
    #endif

    // print the credits
    post( "" );
    post( "||---------------------------------------------------------------------");
    post( "||  P d    S p e c t r a l    T o o l k i t" );
    post( "||---------------------------------------------------------------------");
    post( "||  Version 1.1" );
    post( "||" );
    post( "||  amptodb~ amptomag~ binindex~ binmax~ binmin~ binmix~" );
    post( "||  binmonitor~ binsort~ bintrim~ bitsafe~ blocksmooth~ cartoamp~" );
    post( "||  cartodb~ cartofreq~ cartomag~ cartophase~ cartopolar~ " );
    post( "||  cmplxabs~ cmplxadd~ cmplxdiv~ cmplxmult~ cmplxsqrt~ " );
    post( "||  cmplxsub~ countwrap ctltosig~ dbtoamp~ dbtomag~ degtorad" );
    post( "||  degtoturn~ dspbang~ freqsieve~  freqtocar~ freqtophase~" );
    post( "||  freqtopolar~ fundfreq~ harmprod~ magtoamp~ magtodb~" );
    post( "||  magtrim~ monitor~ oscbank~ pafft~ paifft~ partconv~" );
    post( "||  peaks~ phaseaccum~ phasedelta~ phasetofreq~ piwrap~" );
    post( "||  polartocar~ polartofreq~ radtodeg~ radtoturn~ recip~ rgbtable" );
    post( "||  rotate~ rounder~ scale~ sigtoctl~ softclip~ tabindex~ terminal" );
    post( "||  trunc~ turntodeg~ turntorad~ valleys~ windower winfft~ winifft~" );
    post( "||  !&&~ !=~ !||~ !~ %%~ &&~ <=~ <~ ==~ >=~ >~ ||~" );
    post( "||" );
    post( "||  (c)2019 Cooper Baker" );
    post( "||---------------------------------------------------------------------");
    post( "||  Cross platform encapsulation, build system, and" );
    post( "||  Pd_Spectral_Toolkit library object" );
    post( "||" );
    post( "||  (c)2019 Tom Erbe - UCSD Computer Music" );
    post( "||---------------------------------------------------------------------");
    post( "||  http://www.cooperbaker.com/pd-spectral-toolkit" );
    post( "||---------------------------------------------------------------------");
    post( "" );
}

//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
