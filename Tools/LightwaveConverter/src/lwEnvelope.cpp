#include "lwEnvelope.h"

lwKey *lwEnvelope::addKey( float time, float value )
{
    lwKey *key = new lwKey(time, value);    
    keys.insert(lower_bound(keys.begin(), keys.end(), key), key);
    return key;
}

/*======================================================================
range()
  
Given the value v of a periodic function, returns the equivalent value
v2 in the principal interval [lo, hi].  If i isn't NULL, it receives
the number of wavelengths between v and v2.
    
v2 = v - i * (hi - lo)
    
For example, range( 3 pi, 0, 2 pi, i ) returns pi, with i = 1.
====================================================================== */

float lwEnvelope::range( float v, float lo, float hi, int *i )
{
    float v2, r = hi - lo;
    
    if ( r == 0.0 ) {
        if ( i ) *i = 0;
        return lo;
    }
    
    v2 = lo + v - r * ( float ) floor(( double ) v / r );
    if ( i ) *i = -( int )(( v2 - v ) / r + ( v2 > v ? 0.5 : -0.5 ));
    
    return v2;
}

/*======================================================================
hermite()

Calculate the Hermite coefficients.
====================================================================== */

void lwEnvelope::hermite( float t, float *h1, float *h2, float *h3, float *h4 )
{
    float t2, t3;
    
    t2 = t * t;
    t3 = t * t2;
    
    *h2 = 3.0f * t2 - t3 - t3;
    *h1 = 1.0f - *h2;
    *h4 = t3 - t2;
    *h3 = *h4 - t2 + t;
}

/*======================================================================
bezier()

Interpolate the value of a 1D Bezier curve.
====================================================================== */

float lwEnvelope::bezier( float x0, float x1, float x2, float x3, float t )
{
    float a, b, c, t2, t3;
    
    t2 = t * t;
    t3 = t2 * t;
    
    c = 3.0f * ( x1 - x0 );
    b = 3.0f * ( x2 - x1 ) - c;
    a = x3 - x0 - c - b;
    
    return a * t3 + b * t2 + c * t + x0;
}


/*======================================================================
bez2_time()

Find the t for which bezier() returns the input time.  The handle
endpoints of a BEZ2 curve represent the control points, and these have
(time, value) coordinates, so time is used as both a coordinate and a
parameter for this curve type.
====================================================================== */
  
float lwEnvelope::bez2_time( float x0, float x1, float x2, float x3, float time,    float *t0, float *t1 )
{
    float v, t;
    
    t = *t0 + ( *t1 - *t0 ) * 0.5f;
    v = bezier( x0, x1, x2, x3, t );
    if ( fabs( time - v ) > .0001f ) {
        if ( v > time )
            *t1 = t;
        else
            *t0 = t;
        return bez2_time( x0, x1, x2, x3, time, t0, t1 );
    }
    else
        return t;
}
  
  
  /*
  ======================================================================
  bez2()
  
    Interpolate the value of a BEZ2 curve.
  ====================================================================== */
  
float lwEnvelope::bez2( lwKey *key0, lwKey *key1, float time )
{
    float x, y, t, t0 = 0.0f, t1 = 1.0f;
    
    if ( key0->shape == ID_BEZ2 )
        x = key0->time + key0->param[ 2 ];
    else
        x = key0->time + ( key1->time - key0->time ) / 3.0f;
    
    t = bez2_time( key0->time, x, key1->time + key1->param[ 0 ], key1->time,
        time, &t0, &t1 );
    
    if ( key0->shape == ID_BEZ2 )
        y = key0->value + key0->param[ 3 ];
    else
        y = key0->value + key0->param[ 1 ] / 3.0f;
    
    return bezier( key0->value, y, key1->param[ 1 ] + key1->value, key1->value, t );
}
  
  
  /*
  ======================================================================
  outgoing()
  
    Return the outgoing tangent to the curve at key0.  The value returned
    for the BEZ2 case is used when extrapolating a linear pre behavior and
    when interpolating a non-BEZ2 span.
  ====================================================================== */
  
float lwEnvelope::outgoing( unsigned int key0, unsigned int key1 )
{
    float a, b, d, t, tout;
    
    switch ( keys[key0]->shape )
    {
    case ID_TCB:
        a = ( 1.0f - keys[key0]->tension )
            * ( 1.0f + keys[key0]->continuity )
            * ( 1.0f + keys[key0]->bias );
        b = ( 1.0f - keys[key0]->tension )
            * ( 1.0f - keys[key0]->continuity )
            * ( 1.0f - keys[key0]->bias );
        d = keys[key1]->value - keys[key0]->value;
        
        
        if ( key0 > 0 )
        {
            t = ( keys[key1]->time - keys[key0]->time ) / ( keys[key1]->time - keys[ key0-1 ]->time );
            tout = t * ( a * ( keys[key0]->value - keys[ key0-1 ]->value ) + b * d );
        }
        else
            tout = b * d;
        break;
        
    case ID_LINE:
        d = keys[key1]->value - keys[key0]->value;
        if ( key0 > 0 )
        {
            t = ( keys[key1]->time - keys[key0]->time ) / ( keys[key1]->time - keys[ key0-1 ]->time );
            tout = t * ( keys[key0]->value - keys[ key0-1 ]->value + d );
        }
        else
            tout = d;
        break;
        
    case ID_BEZI:
    case ID_HERM:
        tout = keys[key0]->param[ 1 ];
        
        if ( key0 > 0 )
            tout *= ( keys[key1]->time - keys[key0]->time ) / ( keys[key1]->time - keys[ key0-1 ]->time );
        
        break;
        
    case ID_BEZ2:
        tout = keys[key0]->param[ 3 ] * ( keys[key1]->time - keys[key0]->time );
        if ( fabs( keys[key0]->param[ 2 ] ) > 1e-5f )
            tout /= keys[key0]->param[ 2 ];
        else
            tout *= 1e5f;
        break;
        
    case ID_STEP:
    default:
        tout = 0.0f;
        break;
    }
    
    return tout;
}
  
  
/*======================================================================
incoming()
  
Return the incoming tangent to the curve at key1.  The value returned
for the BEZ2 case is used when extrapolating a linear post behavior.
====================================================================== */
  
float lwEnvelope::incoming( unsigned int key0, unsigned int key1 )
{
    float a, b, d, t, tin;
    
    switch ( keys[key1]->shape )
    {
    case ID_LINE:
        d = keys[key1]->value - keys[key0]->value;
        
        if ( key1 < keys.size()-1 )
        {
            t = ( keys[key1]->time - keys[key0]->time ) / ( keys[ key1+1 ]->time - keys[key0]->time );
            tin = t * ( keys[ key1+1 ]->value - keys[key1]->value + d );
        }
        else
            tin = d;
        
        break;
        
    case ID_TCB:
        a = ( 1.0f - keys[key1]->tension )
            * ( 1.0f - keys[key1]->continuity )
            * ( 1.0f + keys[key1]->bias );
        b = ( 1.0f - keys[key1]->tension )
            * ( 1.0f + keys[key1]->continuity )
            * ( 1.0f - keys[key1]->bias );
        d = keys[key1]->value - keys[key0]->value;
        if ( key1 < keys.size()-1 ) {
            t = ( keys[key1]->time - keys[key0]->time ) / ( keys[ key1+1 ]->time - keys[key0]->time );
            tin = t * ( b * ( keys[ key1+1 ]->value - keys[key1]->value ) + a * d );
        }
        else
            tin = a * d;
        break;
        
    case ID_BEZI:
    case ID_HERM:
        tin = keys[key1]->param[ 0 ];
        if ( key1 < keys.size()-1 )
            tin *= ( keys[key1]->time - keys[key0]->time ) / ( keys[ key1+1 ]->time - keys[key0]->time );
        break;
        return tin;
        
    case ID_BEZ2:
        tin = keys[key1]->param[ 1 ] * ( keys[key1]->time - keys[key0]->time );
        if ( fabs( keys[key1]->param[ 0 ] ) > 1e-5f )
            tin /= keys[key1]->param[ 0 ];
        else
            tin *= 1e5f;
        break;
        
    case ID_STEP:
    default:
        tin = 0.0f;
        break;
    }
    
    return tin;
}

/*======================================================================
evalEnvelope()

Given a list of keys and a time, returns the interpolated value of the
envelope at that time.
====================================================================== */
  
float lwEnvelope::evaluate( float time )
{
    lwKey *key0, *key1, *skey, *ekey;
    float t, h1, h2, h3, h4, tin, tout, offset = 0.0f;
    int noff;
    int key0index, key1index;
    
    
    /* if there's no key, the value is 0 */
    
    if ( keys.size() == 0 ) return 0.0f;
    
    /* if there's only one key, the value is constant */
    
    if ( keys.size() == 1 ) return keys[0]->value;
    
    /* find the first and last keys */
    
    key0index = 0;
    key1index = keys.size()-1;
    skey = keys[key0index];
    ekey = keys[key1index];
    
    /* use pre-behavior if time is before first key time */
    
    if ( time < skey->time )
    {
        switch ( behavior[ 0 ] )
        {
        case BEH_RESET:
            return 0.0f;
            
        case BEH_CONSTANT:
            return skey->value;
            
        case BEH_REPEAT:
            time = range( time, skey->time, ekey->time, NULL );
            break;
            
        case BEH_OSCILLATE:
            time = range( time, skey->time, ekey->time, &noff );
            if ( noff % 2 )
                time = ekey->time - skey->time - time;
            break;
            
        case BEH_OFFSET:
            time = range( time, skey->time, ekey->time, &noff );
            offset = noff * ( ekey->value - skey->value );
            break;
            
        case BEH_LINEAR:
            tout = outgoing( key0index, key0index+1 ) / ( keys[key0index+1]->time - keys[key0index]->time );

            return tout * ( time - skey->time ) + skey->value;
        }
    }
    
    /* use post-behavior if time is after last key time */
    
    else if ( time > ekey->time ) {
        switch ( behavior[ 1 ] )
        {
        case BEH_RESET:
            return 0.0f;
            
        case BEH_CONSTANT:
            return ekey->value;
            
        case BEH_REPEAT:
            time = range( time, skey->time, ekey->time, NULL );
            break;
            
        case BEH_OSCILLATE:
            time = range( time, skey->time, ekey->time, &noff );
            if ( noff % 2 )
                time = ekey->time - skey->time - time;
            break;
            
        case BEH_OFFSET:
            time = range( time, skey->time, ekey->time, &noff );
            offset = noff * ( ekey->value - skey->value );
            break;
            
        case BEH_LINEAR:
            tin = incoming( key1index-1, key1index ) / ( ekey->time - keys[key1index-1]->time );
            return tin * ( time - ekey->time ) + ekey->value;
        }
    }
    
    /* get the endpoints of the interval being evaluated */
    
    key0index = keys.size()-2;
    key1index = keys.size()-1;
    key0 = keys[key0index];
    key1 = keys[key1index];
    
    /* check for singularities first */
    
    if ( time == key0->time )
        return key0->value + offset;
    else if ( time == key1->time )
        return key1->value + offset;
    
    /* get interval length, time in [0, 1] */
    
    t = ( time - key0->time ) / ( key1->time - key0->time );
    
    /* interpolate */
    
    switch ( key1->shape )
    {
    case ID_TCB:
    case ID_BEZI:
    case ID_HERM:
        tout = outgoing( key0index, key1index );
        tin = incoming( key0index, key1index );
        hermite( t, &h1, &h2, &h3, &h4 );
        return h1 * key0->value + h2 * key1->value + h3 * tout + h4 * tin + offset;
        
    case ID_BEZ2:
        return bez2( key0, key1, time ) + offset;
        
    case ID_LINE:
        return key0->value + t * ( key1->value - key0->value ) + offset;
        
    case ID_STEP:
        return key0->value + offset;
        
    default:
        return offset;
    }
}
