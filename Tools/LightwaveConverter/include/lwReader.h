#ifndef _LWREADER_H_
#define _LWREADER_H_

#include "lwo.h"
#include "lwObject.h"
#include <istream>
#include <vector>

class lwReader
{
public:
    lwObject *readObjectFromFile( const char *nfilename);
    lwObject *readObjectFromStream( istream *nis);
private:
    istream *is;
    unsigned long chunksize;
    unsigned long currentchunkid;
    unsigned long formsize;
    unsigned long flen;

    bool *flags;

    lwObject *lwGetLWLO();
    lwObject *lwGetLWO2();

    int lwGetPoints( vpoints &points );
    int lwGetPolygons( vpolygons &polygons, int ptoffset );
    int lwGetLWOBPolygons( vpolygons &polygons, int ptoffset );
    int lwGetTags( vtags &tags );
    int lwGetPolygonTags( vtags &tags, int tagsoffset, vpolygons &polygons, int polygonsoffset );
    lwVMap *lwGetVMap( int ptoffset, int poloffset, int perpoly );
    lwClip *lwGetClip();
    lwEnvelope *lwGetEnvelope();
    int lwGetTHeader( int hsz, lwTexture &tex );
    int lwGetTMap( int tmapsz, lwTMap &tmap );
    lwImageMap *lwGetImageMap( int rsz, lwTexture &tex );
    lwProcedural *lwGetProcedural( int rsz, lwTexture &tex );
    lwGradient *lwGetGradient( int rsz, lwTexture &tex );
    lwTexture *lwGetTexture( int bloksz, unsigned int type );
    lwPlugin *lwGetShader( int bloksz );
    lwSurface *lwGetSurface();
    lwSurface *lwGetLWOBSurface( lwObject *obj );

    int add_clip( char *s, vclips &clips );
    int add_tvel( float pos[], float vel[], venvelopes &envelopes );
    lwTexture *get_texture( char *s );

    char *getbytes( int size );
    void skipbytes( int n );
    short getI2();
    long getI4();
    unsigned char getU1();
    unsigned short getU2();
    unsigned long getU4();
    int   getVX();
    float getF4();
    char *getS0();
    short sgetI2( char **bp );
    long  sgetI4( char **bp );
    unsigned char  sgetU1( char **bp );
    unsigned short sgetU2( char **bp );
    unsigned long  sgetU4( char **bp );
    int   sgetVX( char **bp );
    float sgetF4( char **bp );
    char *sgetS0( char **bp );

    unsigned short swappedShort(unsigned short w);
    unsigned long swappedLong(unsigned long w);
    void revbytes( void *bp, int elsize, int elcount );
};

#endif // _LWREADER_H_

