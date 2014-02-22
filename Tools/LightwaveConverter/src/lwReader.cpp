#include "lwReader.h"

lwObject *lwReader::readObjectFromFile( const char *nfilename)
{
    lwObject *nobject = NULL;

    ifstream *ifs = new ifstream();
    try
    {
        ifs->open(nfilename, ios::in|ios::binary);
        if (ifs->fail())
            cout << "Could not open file: " << nfilename << endl;
        else
            nobject = readObjectFromStream( ifs );
    }
    catch (...)
    {
        if (nobject) delete nobject;
        nobject = NULL;
    }
    ifs->close();
    delete ifs;
    return nobject;
}

lwObject *lwReader::readObjectFromStream( istream *nis)
{
    lwObject *nobject = 0;
    chunksize = 0;
    currentchunkid = 0;
    formsize = 0;
    flen = 0;

    is = nis;

    try
    {
        long id = getU4();
        if ( id == ID_FORM )
        {
            formsize = getU4();
            long type = getU4();
            
            switch(type)
            {
            case ID_LWO2:
                nobject = lwGetLWO2(); // FORM0000LWXX -> filelength - 12
                break;
            case ID_LWOB:
            case ID_LWLO:
                nobject = lwGetLWLO(); // FORM0000LWXX -> filelength - 12
                break;
            default:
                throw "File does not contain Lightwave object.";
            }
        }
        else
            throw "Not an IFF FORM file.";
    }
    catch (char *errstr)
    {
        cout << "Error near byte " << is->tellg() << " in chunk " << currentchunkid << " : " << errstr << endl;
    }
    catch (...)
    {
        if (nobject) delete nobject;
        nobject = NULL;
    }
    return nobject;
}


lwObject *lwReader::lwGetLWLO()
{
    long filepos = is->tellg();
    if (filepos == -1) return NULL;
    long formstart = filepos;
    long formend = filepos + formsize; // FORM0000LWXX -> filelength - 12

    lwObject *object = new lwObject;
    if ( !object ) return NULL;

    lwLayer *layer = new lwLayer;
    if ( !layer ) goto Fail;

    lwSurface *surface;
    unsigned int rlen;
    unsigned long i;
    while ( filepos < formend )
    {
        currentchunkid = getU4();
        chunksize = getU4();
        chunksize += chunksize & 1;

        switch ( currentchunkid )
        {
        case ID_SRFS:
            if ( !lwGetTags(object->tags ))
                goto Fail;
            break;
        case ID_LAYR:
            if ( object->layers.size() > 0 )
            {
                layer = new lwLayer;
                if ( !layer ) goto Fail;
            }
            object->layers.push_back(layer);


            flen = 0;
            layer->index = getU2();
            layer->flags = getU2();
            layer->name = getS0();

            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) goto Fail;
            if ( rlen < chunksize )
                is->seekg(chunksize - rlen, ios_base::cur);
            break;
        case ID_PNTS:
            if ( !lwGetPoints(layer->points ))
                goto Fail;
            break;

        case ID_POLS:
            if ( !lwGetLWOBPolygons(layer->polygons, layer->pointsoffset ))
                goto Fail;
            break;

        case ID_SURF:
            surface = lwGetLWOBSurface(object );
            if ( surface ) object->surfaces.push_back( surface );
            break;

        case ID_SHCP:
        default:
            is->seekg(chunksize, ios_base::cur);
            break;
        }

        /* end of the file? */

        filepos = is->tellg();
        if ( filepos == -1 ) break;
        if ( filepos > formend ) break; // read too much
    }

    if ( object->layers.size() == 0 )
        object->layers.push_back(layer);

    for (i = 0; i < object->layers.size(); i++)
    {
        layer = object->layers[i];
        layer->lwGetBoundingBox();
        layer->lwResolveVertexPoints();
        layer->calculatePolygonNormals();
        layer->lwGetPointPolygons();

        if ( !layer->lwResolvePolySurfaces(object->surfaces, object->tags)) goto Fail;
        layer->calculateVertexNormals();
    }

    return object;
Fail:
    if (object) delete object;
    return NULL;
}

lwObject *lwReader::lwGetLWO2()
{
    long filepos = is->tellg();
    if (filepos == -1) return NULL;
    long formstart = filepos;
    long formend = filepos + formsize; // FORM0000LWXX -> filelength - 12

    /* allocate an object and a default layer */

    lwObject *object = new lwObject;
    if ( !object ) return NULL;

    lwLayer *layer = new lwLayer;
    if ( !layer ) goto Fail;

    unsigned int i, rlen;

    lwVMap *vmap;
    lwEnvelope *envelope;
    lwClip *clip;
    lwSurface *surface;

    /* process chunks as they're encountered */

    while ( filepos < formend )
    {
        currentchunkid = getU4();
        chunksize = getU4();
        chunksize += chunksize & 1;

        switch ( currentchunkid )
        {
        case ID_LAYR:
            if ( object->layers.size() > 0 )
            {
                layer = new lwLayer;
                if ( !layer ) goto Fail;
            }
            object->layers.push_back(layer);

            flen = 0;
            layer->index = getU2();
            layer->flags = getU2();
            layer->pivot.x = getF4();
            layer->pivot.y = getF4();
            layer->pivot.z = getF4();
            layer->name = getS0();

            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) goto Fail;
            if ( rlen <= chunksize - 2 )
                layer->parent = getU2();
            rlen = flen;
            if ( rlen < chunksize )
                is->seekg(chunksize - rlen, ios_base::cur);
            break;

        case ID_PNTS:
            if ( !lwGetPoints(layer->points ))
                goto Fail;
            break;

        case ID_POLS:
            if ( !lwGetPolygons(layer->polygons, layer->pointsoffset ))
                goto Fail;
            break;

        case ID_VMAP:
        case ID_VMAD:
            vmap = lwGetVMap(layer->pointsoffset, layer->polygonsoffset, currentchunkid == ID_VMAD );
            if ( !vmap ) goto Fail;
            layer->vmaps.push_back(vmap);
            break;

        case ID_PTAG:
            if ( !lwGetPolygonTags(object->tags, object->tagsoffset, layer->polygons, layer->polygonsoffset ))
                goto Fail;
            break;

        case ID_BBOX:
            flen = 0;
            layer->bboxmin.x = getF4();
            layer->bboxmin.y = getF4();
            layer->bboxmin.z = getF4();
            layer->bboxmax.x = getF4();
            layer->bboxmax.y = getF4();
            layer->bboxmax.z = getF4();
            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) goto Fail;
            if ( rlen < chunksize )
                is->seekg(chunksize - rlen, ios_base::cur );
            break;

        case ID_TAGS:
            if ( !lwGetTags(object->tags ))
                goto Fail;
            break;

        case ID_ENVL:
            envelope = lwGetEnvelope();
            if ( !envelope ) goto Fail;
            object->envelopes.push_back( envelope );
            break;

        case ID_CLIP:
            clip = lwGetClip();
            if ( !clip ) goto Fail;
            object->clips.push_back( clip );
            break;

        case ID_SURF:
            surface = lwGetSurface();
            if ( !surface ) goto Fail;
            object->surfaces.push_back( surface );
            break;

        case ID_DESC:
        case ID_TEXT:
        case ID_ICON:
        default:
            is->seekg(chunksize, ios_base::cur );
            break;
        }

        /* end of the file? */
        filepos = is->tellg();
        if ( filepos == -1 ) break;
        if ( filepos > formend ) break; // read too much
    }
    
    if ( object->layers.size() == 0 )
        object->layers.push_back(layer);
                
    for (i = 0; i < object->layers.size(); i++)
    {
        layer = object->layers[i];
        if ( !layer->lwResolvePolySurfaces(object->surfaces, object->tags)) goto Fail;

        layer->lwGetBoundingBox();
        layer->lwResolveVertexPoints();
        layer->calculatePolygonNormals();
        layer->lwGetPointPolygons();
        layer->calculateVertexNormals();
        layer->lwGetPointVMaps();
        layer->lwGetPolyVMaps();
    }

    return object;
Fail:
    if(object) delete object;
    return NULL;
}

#define FLEN_ERROR INT_MIN

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_LINUX
/*
=====================================================================
revbytes()

Reverses byte order in place.
 
INPUTS
bp       bytes to reverse
elsize  size of the underlying data type
elcount  number of elements to swap

RESULTS
Reverses the byte order in each of elcount elements.

This only needs to be defined on little-endian platforms, most
notably Windows.  lwo2.h replaces this with a #define on big-endian
platforms.
===================================================================== */

unsigned short lwReader::swappedShort(unsigned short w)
{
    unsigned short tmp;
    tmp =  (w & 0x00ff);
    tmp = ((w & 0xff00) >> 0x08) | (tmp << 0x08);
    return tmp;
}

unsigned long lwReader::swappedLong(unsigned long w)
{
    unsigned long tmp;
    tmp =  (w & 0x000000ff);
    tmp = ((w & 0x0000ff00) >> 0x08) | (tmp << 0x08);
    tmp = ((w & 0x00ff0000) >> 0x10) | (tmp << 0x08);
    tmp = ((w & 0xff000000) >> 0x18) | (tmp << 0x08);
    return tmp;
}

void lwReader::revbytes( void *bp, int elsize, int elcount )
{
    register char *p, *q;
    
    p = ( char * ) bp;
    
    if ( elsize == 2 ) {
        q = p + 1;
        while ( elcount-- ) {
            *p ^= *q;
            *q ^= *p;
            *p ^= *q;
            p += 2;
            q += 2;
        }
        return;
    }
    
    while ( elcount-- ) {
        q = p + elsize - 1;
        while ( p < q ) {
            *p ^= *q;
            *q ^= *p;
            *p ^= *q;
            ++p;
            --q;
        }
        p += elsize >> 1;
    }
}
#endif

char *lwReader::getbytes( int size )
{
    char *data;
    
    if ( flen == FLEN_ERROR ) return NULL;
    if ( size < 0 ) {
        flen = FLEN_ERROR;
        return NULL;
    }
    if ( size == 0 ) return NULL;

    data = (char*)malloc(size);

    if ( !data )
    {
        flen = FLEN_ERROR;
        return NULL;
    }
    
    is->read((char *)data, size);
    if (is->gcount() != size)
    {
        flen = FLEN_ERROR;
        free (data);
        return NULL;
    }
    
    flen += size;
    return data;
}


void lwReader::skipbytes( int n )
{
    if ( flen == FLEN_ERROR ) return;
    
    is->seekg(n, ios_base::cur);
    
    if (is->bad())
        flen = FLEN_ERROR;
    else
        flen += n;
}

short lwReader::getI2()
{
    short i;
    
    if ( flen == FLEN_ERROR ) return 0;
    
    is->read((char *)&i, 2);
    if (is->gcount() != 2)
    {
        flen = FLEN_ERROR;
        return 0;
    }
    revbytes( &i, 2, 1 );
    flen += 2;
    return i;
}


long lwReader::getI4()
{
    long i;
    
    if ( flen == FLEN_ERROR ) return 0;
    
    is->read((char *)&i, 4);
    if (is->gcount() != 4) {
        flen = FLEN_ERROR;
        return 0;
    }
    revbytes( &i, 4, 1 );
    flen += 4;
    return i;
}


unsigned char lwReader::getU1()
{
    unsigned char i;
    
    if ( flen == FLEN_ERROR ) return 0;
    is->read((char *)&i, 1);
    if (is->gcount() != 1)
    {
        flen = FLEN_ERROR;
        return 0;
    }
    flen += 1;
    return i;
}


unsigned short lwReader::getU2()
{
    unsigned short i;
    
    if ( flen == FLEN_ERROR ) return 0;
    is->read((char *)&i, 2);
    if (is->gcount() != 2)
    {
        flen = FLEN_ERROR;
        return 0;
    }
    revbytes( &i, 2, 1 );
    flen += 2;
    return i;
}


unsigned long lwReader::getU4()
{
    unsigned long i;
    
    if ( flen == FLEN_ERROR ) return 0;
    is->read((char *)&i, 4);
    if (is->gcount() != 4)
    {
        flen = FLEN_ERROR;
        return 0;
    }
    revbytes( &i, 4, 1 );
    flen += 4;
    return i;
}


int lwReader::getVX()
{
    int i;
    short c;
    
    if ( flen == FLEN_ERROR ) return 0;
    
    is->read((char *)&c, 2);
    
    if ( (c & 0x00FF) != 0xFF )
    {
        i = swappedShort(c);
        flen += 2;
    }
    else
    {
        i = (swappedShort(c) & 0x00FF) << 16;
        is->read((char *)&c, 2);
        i |= swappedShort(c);
        flen += 4;
    }
    
    if ( is->bad() ) {
        flen = FLEN_ERROR;
        return 0;
    }
    return i;
}


float lwReader::getF4()
{
    float f;
    
    if ( flen == FLEN_ERROR ) return 0.0f;
    is->read((char *)&f, 4);
    if (is->gcount() != 4)
    {
        flen = FLEN_ERROR;
        return 0.0f;
    }
    revbytes( &f, 4, 1 );
    flen += 4;
    return f;
}

char *lwReader::getS0()
{
    char *s;
    int i, len, pos;
    char c;

    if ( flen == FLEN_ERROR ) return NULL;

    pos = is->tellg();
    if (pos == -1) return 0;

    i = 0;
    do
    {
        is->read(&c, 1);
        i ++;
    }
    while (c > 0);

    if ( i == 1 ) // word align
    {
        is->read(&c, 1);
        flen += 2;
        return NULL;
    }

    len = i + ( i & 1 );

    s = (char *)malloc(len);
    if ( !s )
    {
        flen = FLEN_ERROR;
        return NULL;
    }

    is->seekg(pos, ios_base::beg);
    if (is->bad())
    {
        flen = FLEN_ERROR;
        return NULL;
    }

    is->read(s, len);
    if (is->gcount() != len)
    {
        flen = FLEN_ERROR;
        return NULL;
    }
    
    flen += len;
    return s;
}

short lwReader::sgetI2( char **bp )
{
    short i;
    
    if ( flen == FLEN_ERROR ) return 0;
    memcpy( &i, *bp, 2 );
    revbytes( &i, 2, 1 );
    flen += 2;
    *bp += 2;
    return i;
}


long lwReader::sgetI4( char **bp )
{
    long i;
    
    if ( flen == FLEN_ERROR ) return 0;
    memcpy( &i, *bp, 4 );
    revbytes( &i, 4, 1 );
    flen += 4;
    *bp += 4;
    return i;
}


unsigned char lwReader::sgetU1( char **bp )
{
    unsigned char c;
    
    if ( flen == FLEN_ERROR ) return 0;
    c = **bp;
    flen += 1;
    *bp++;
    return c;
}

unsigned short lwReader::sgetU2(char **bp )
{
    unsigned char *buf = (unsigned char *)*bp;
    unsigned short i;
    
    if ( flen == FLEN_ERROR ) return 0;
    i = ( buf[ 0 ] << 8 ) | buf[ 1 ];
    flen += 2;
    *bp += 2;
    return i;
}

unsigned long lwReader::sgetU4( char **bp )
{
    unsigned long i;
    
    if ( flen == FLEN_ERROR ) return 0;
    memcpy( &i, *bp, 4 );
    revbytes( &i, 4, 1 );
    flen += 4;
    *bp += 4;
    return i;
}

int lwReader::sgetVX( char **bp )
{
    unsigned char *buf = (unsigned char *)*bp;
    int i;
    
    if ( flen == FLEN_ERROR ) return 0;
    
    if ( buf[ 0 ] != 0xFF )
    {
        i = buf[ 0 ] << 8 | buf[ 1 ];
        flen += 2;
        *bp += 2;
    }
    else
    {
        i = ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
        flen += 4;
        *bp += 4;
    }
    return i;
}


float lwReader::sgetF4( char **bp )
{
    float f;
    
    if ( flen == FLEN_ERROR ) return 0.0f;
    memcpy( &f, *bp, 4 );
    revbytes( &f, 4, 1 );
    flen += 4;
    *bp += 4;
    return f;
}


char *lwReader::sgetS0( char **bp )
{
    char *s;
    const char *buf = (const char *)*bp;
    unsigned int len;
    
    if ( flen == FLEN_ERROR ) return NULL;
    
    len = strlen( buf ) + 1;
    if ( len == 1 ) {
        flen += 2;
        *bp += 2;
        return NULL;
    }
    len += len & 1;
    s = (char *)malloc(len);
    if ( !s )
    {
        flen = FLEN_ERROR;
        return NULL;
    }
    
    memcpy( s, buf, len );
    flen += len;
    *bp += len;
    return s;
}

lwClip *lwReader::lwGetClip()
{
    lwClip *clip;
    lwPlugin *filt;
    unsigned int id;
    unsigned short sz;
    unsigned int pos, rlen;
    
    
    /* allocate the Clip structure */
    
    clip = new lwClip;
    if ( !clip ) goto Fail;
    
    /* remember where we started */
    
    flen = 0;
    pos = is->tellg();
    
    /* index */
    
    clip->index = getI4();
    
    /* first subchunk header */
    
    clip->type = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    sz += sz & 1;
    flen = 0;

    switch ( clip->type )
    {
    case ID_STIL:
        {
            lwClipStill *still = new lwClipStill;
            still->name = getS0();
            clip->source.still = still;
        }
        break;
        
    case ID_ISEQ:
        {
            lwClipSeq *seq = new lwClipSeq;
            seq->digits  = getU1();
            seq->flags   = getU1();
            seq->offset  = getI2();
            getU2();  /* reserved */
            seq->start   = getI2();
            seq->end     = getI2();
            seq->prefix  = getS0();
            seq->suffix  = getS0();
            clip->source.seq = seq;
        }
        break;
        
    case ID_ANIM:
        {
            lwClipAnim *anim = new lwClipAnim;
            anim->name = getS0();
            anim->server = getS0();
            rlen = flen;
            anim->data = getbytes( sz - rlen );
            clip->source.anim = anim;
        }
        break;
        
    case ID_XREF:
        {
            lwClipXRef *xref = new lwClipXRef;
            xref->index  = getI4();
            xref->string = getS0();
            clip->source.xref = xref;
        }
        break;
        
    case ID_STCC:
        {
            lwClipCycle *cycle = new lwClipCycle;
            cycle->lo   = getI2();
            cycle->hi   = getI2();
            cycle->name = getS0();
            clip->source.cycle = cycle;
        }
        break;
        
    default:
        break;
    }
    
    /* error while reading current subchunk? */
    
    rlen = flen;
    if ( rlen < 0 || rlen > sz ) goto Fail;
    
    /* skip unread parts of the current subchunk */
    
    if ( rlen < sz )
        is->seekg(sz - rlen, ios_base::cur );
    
    /* end of the CLIP chunk? */
    
    rlen = is->tellg();
    rlen -= pos;
    if ( chunksize < rlen ) goto Fail;
    if ( chunksize == rlen )
        return clip;
    
    /* process subchunks as they're encountered */
    
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_TIME:
            clip->start_time = getF4();
            clip->duration = getF4();
            clip->frame_rate = getF4();
            break;
            
        case ID_CONT:
            clip->contrast.val = getF4();
            clip->contrast.eindex = getVX();
            break;
            
        case ID_BRIT:
            clip->brightness.val = getF4();
            clip->brightness.eindex = getVX();
            break;
            
        case ID_SATR:
            clip->saturation.val = getF4();
            clip->saturation.eindex = getVX();
            break;
            
        case ID_HUE:
            clip->hue.val = getF4();
            clip->hue.eindex = getVX();
            break;
            
        case ID_GAMM:
            clip->gamma.val = getF4();
            clip->gamma.eindex = getVX();
            break;
            
        case ID_NEGA:
            clip->negative = getU2();
            break;
            
        case ID_IFLT:
        case ID_PFLT:
            filt = new lwPlugin;
            if ( !filt ) goto Fail;
            
            filt->name = getS0();
            filt->flags = getU2();
            rlen = flen;
            filt->data = getbytes( sz - rlen );
            
            if ( id == ID_IFLT )
                clip->ifilters.push_back(filt);
            else
                clip->pfilters.push_back(filt);

            break;
            
        default:
            break;
        }
        
        /* error while reading current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) goto Fail;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the CLIP chunk? */
        
        rlen = is->tellg();
        rlen -= pos;
        if ( chunksize < rlen ) goto Fail;
        if ( chunksize == rlen ) break;
        
        /* get the next chunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) goto Fail;
    }
    
    return clip;
    
Fail:
    delete clip;
    return NULL;
}

lwEnvelope *lwReader::lwGetEnvelope()
{
    lwEnvelope *env;
    lwKey *key;
    lwPlugin *plug;
    unsigned int id;
    unsigned short sz;
    float f[ 4 ];
    int i, nparams, pos;
    unsigned int rlen;
    
    /* allocate the Envelope structure */
    
    env = new lwEnvelope;
    if ( !env ) goto Fail;
    
    /* remember where we started */
    
    flen = 0;
    pos = is->tellg();
    
    /* index */
    
    env->index = getVX();
    
    /* first subchunk header */
    
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    /* process subchunks as they're encountered */
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_TYPE:
            env->type = getU2();
            break;
            
        case ID_NAME:
            env->name = getS0();
            break;
            
        case ID_PRE:
            env->behavior[ 0 ] = getU2();
            break;
            
        case ID_POST:
            env->behavior[ 1 ] = getU2();
            break;
            
        case ID_KEY:
            key = env->addKey(getF4(), getF4());
            break;
            
        case ID_SPAN:
            if ( !key ) goto Fail;
            key->shape = getU4();
            
            nparams = ( sz - 4 ) / 4;
            if ( nparams > 4 ) nparams = 4;
            for ( i = 0; i < nparams; i++ )
                f[ i ] = getF4();
            
            switch ( key->shape ) {
            case ID_TCB:
                key->tension = f[ 0 ];
                key->continuity = f[ 1 ];
                key->bias = f[ 2 ];
                break;
                
            case ID_BEZI:
            case ID_HERM:
            case ID_BEZ2:
                for ( i = 0; i < nparams; i++ )
                    key->param[ i ] = f[ i ];
                break;
            }
            break;
            
            case ID_CHAN:
                plug = new lwPlugin;
                if ( !plug ) goto Fail;
                
                plug->name = getS0();
                plug->flags = getU2();
                plug->data = getbytes( sz - flen );
                
                env->cfilters.push_back( plug );
                env->ncfilters++;
                break;
                
            default:
                break;
        }
        
        /* error while reading current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) goto Fail;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the ENVL chunk? */
        
        rlen = is->tellg();
        rlen -= pos;
        if ( chunksize < rlen ) goto Fail;
        if ( chunksize == rlen ) break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) goto Fail;
    }
    
    return env;
    
Fail:
    delete env;
    return NULL;
}

/*
======================================================================
lwGetPoints()

  Read point records from a PNTS chunk in an LWO2 file.  The points are
  added to the array in the lwPointags.
====================================================================== */

int lwReader::lwGetPoints( vpoints &points )
{
    if ( chunksize == 1 ) return 1;

    int npoints;

    npoints = chunksize / 12;

    int rpoints = npoints >= 4096 ? 4096 : npoints; 
    float *f = (float *)malloc(rpoints * 3 * sizeof(float));

    while(npoints > 0)
    {
        is->read((char *)f, rpoints * 3 * sizeof(float));
        revbytes( f, sizeof(float), rpoints * 3 );
        for (int i = 0 ; i < rpoints * 3; i += 3 )
            points.push_back(new lwPoint(f[ i ], f[ i + 1 ], f[ i + 2 ]));

        npoints -= rpoints;
        rpoints = npoints >= 4096 ? 4096 : npoints; 
    }

    free(f);
    return 1;
}

/*
======================================================================
lwGetPolygons5()

Read polygon records from a POLS chunk in an LWOB file. The polygons
are added to the array in the vpolygons.
======================================================================*/

int lwReader::lwGetLWOBPolygons( vpolygons &polygons, int ptoffset )
{
    lwPolygon *polygon;

    char *buf, *bp;
    int i, j, k, nv, nvertices, npolygons;

    if ( chunksize == 0 ) return 1;

    /* read the whole chunk */

    flen = 0;
    buf = getbytes( chunksize );
    if ( !buf ) goto Fail;

    /* count the polygons and vertices */

    nvertices = 0;
    npolygons = 0;
    bp = buf;

    while ( bp < buf + chunksize )
    {
        nv = sgetU2( &bp );
        nvertices += nv;
        npolygons++;
        bp += 2 * nv;
        i = sgetI2( &bp );
        if ( i < 0 ) bp += 2;       /* detail polygons */
    }

    k = 0;
    bp = buf;

    for ( i = 0; i < npolygons; i++ )
    {
        polygon = new lwPolygon;
        nv = sgetU2( &bp );

        polygon->type = ID_FACE;
        for ( j = 0; j < nv; j++ )
            polygon->vertices.push_back(new lwVertex(sgetU2( &bp ) + ptoffset));

        j = sgetI2( &bp );
        if ( j < 0 ) {
            j = -j;
            bp += 2;
        }
        j -= 1;

        polygon->surfidx = j;
        polygons.push_back(polygon);
    }

    free(buf);
    return 1;

Fail:
    free(buf);
    return 0;
}

int lwReader::lwGetPolygons( vpolygons &polygons, int ptoffset )
{
    lwPolygon *polygon;
    char *buf, *bp;

    int i, j, flags, nv, nvertices, npolygons;
    unsigned int type;  
    
    if ( chunksize == 0 ) return 1;
    
    /* read the whole chunk */
    
    flen = 0;
    type = getU4();
    buf = getbytes( chunksize - 4 );
    if ( chunksize != flen ) goto Fail;
    
    /* count the polygons and vertices */
    
    nvertices = 0;
    npolygons = 0;
    bp = buf;
    
    while ( bp < buf + chunksize - 4 )
    {
        nv = sgetU2( &bp );
        nv &= 0x03FF;
        nvertices += nv;
        npolygons++;
        for ( i = 0; i < nv; i++ )
            j = sgetVX( &bp );
    }
    
    /* fill in the new polygons */

    bp = buf;
    
    for ( i = 0; i < npolygons; i++ )
    {
        nv = sgetU2( &bp );
        flags = nv & 0xFC00;
        nv &= 0x03FF;
        
        polygon = new lwPolygon;
        
        polygon->flags = flags;
        polygon->type = type;

        for (j = 0; j < nv; j++ )
            polygon->vertices.push_back(new lwVertex( sgetVX( &bp ) + ptoffset ));

        polygons.push_back(polygon);
    }
    
    free(buf);
    return 1;
    
Fail:
    free(buf);
    return 0;
}

/*
======================================================================
lwGetTags()

  Read tag strings from a TAGS chunk in an LWO2 file.  The tags are
  added to the lwTag array.
====================================================================== */

int lwReader::lwGetTags( vtags &tags )
{
    char *buf, *bp;
    
    if ( chunksize == 0 ) return 1;
    
    /* read the whole chunk */
    
    flen = 0;
    buf = getbytes( chunksize );
    if ( !buf ) return 0;
    
    bp = buf;
    while ( bp < buf + chunksize )
        tags.push_back( sgetS0( &bp ));

    free(buf);
    return 1;
}


/*
======================================================================
lwGetPolygonTags()

  Read polygon tags from a PTAG chunk in an LWO2 file.
====================================================================== */

int lwReader::lwGetPolygonTags( vtags &tags, int tagsoffset, vpolygons &polygons, int polygonsoffset )
{
    unsigned int type;
    unsigned int rlen;
    int i, j;
    
    flen = 0;
    type = getU4();
    rlen = flen;
    if ( rlen < 0 ) return 0;
    
    if ( type != ID_SURF && type != ID_PART && type != ID_SMGP ) {
        is->seekg(chunksize - 4, ios_base::cur );
        return 1;
    }
    
    switch ( type )
    {
    case ID_SURF:
        while ( rlen < chunksize )
        {
            i = getVX() + polygonsoffset;
            j = getVX() + tagsoffset;
            
            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) return 0;
            
            polygons[ i ]->surface = 0;
            polygons[ i ]->surfidx = j;
        }
        break;
    case ID_PART:
        while ( rlen < chunksize )
        {
            i = getVX() + polygonsoffset;
            j = getVX() + tagsoffset;
            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) return 0;
            
            polygons[ i ]->part = j;
            
        }
        break;
    case ID_SMGP:
        while ( rlen < chunksize )
        {
            i = getVX() + polygonsoffset;
            j = getVX() + tagsoffset;
            rlen = flen;
            if ( rlen < 0 || rlen > chunksize ) return 0;
            
            polygons[ i ]->smoothgrp = j;
            
        }
        break;
    }
    
    
    return 1;
}

/*
======================================================================
lwGetTHeader()

  Read a texture map header from a SURF.BLOK in an LWO2 file.  This is
  the first subchunk in a BLOK, and its contents are common to all three
  texture types.
  ======================================================================
*/

int lwReader::lwGetTHeader( int hsz, lwTexture &tex )
{
    unsigned int id;
    unsigned short sz;
    int pos, rlen;
    
    /* remember where we started */
    
    flen = 0;
    pos = is->tellg();
    
    /* ordinal string */
    
    tex.ord = getS0();
    
    /* first subchunk header */
    
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) return 0;
    
    /* process subchunks as they're encountered */
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_CHAN:
            tex.chan = getU4();
            break;
            
        case ID_OPAC:
            tex.opac_type = getU2();
            tex.opacity.val = getF4();
            tex.opacity.eindex = getVX();
            break;
            
        case ID_ENAB:
            tex.enabled = getU2();
            break;
            
        case ID_NEGA:
            tex.negative = getU2();
            break;
            
        case ID_AXIS:
            tex.axis = getU2();
            break;
            
        default:
            break;
        }
        
        /* error while reading current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) return 0;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the texture header subchunk? */
        
        int fpos = is->tellg();
        if ( fpos == -1 ) break;
        if ( hsz + pos <= fpos ) break;

//      if ( hsz + pos <= is->tellg())
//          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) return 0;
    };
    
    int g = is->tellg(); 
    flen = g - pos;
    return 1;
}

/*======================================================================
lwGetTMap()

  Read a texture map from a SURF.BLOK in an LWO2 file.  The TMAP
  defines the mapping from texture to world or object coordinates.
====================================================================== */

int lwReader::lwGetTMap( int tmapsz, lwTMap &tmap )
{
    unsigned int id;
    unsigned short sz;
    int rlen, pos, i;
    
    pos = is->tellg();
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) return 0;
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_SIZE:
            for ( i = 0; i < 3; i++ )
                tmap.size.val[ i ] = getF4();
            tmap.size.eindex = getVX();
            break;
            
        case ID_CNTR:
            for ( i = 0; i < 3; i++ )
                tmap.center.val[ i ] = getF4();
            tmap.center.eindex = getVX();
            break;
            
        case ID_ROTA:
            for ( i = 0; i < 3; i++ )
                tmap.rotate.val[ i ] = getF4();
            tmap.rotate.eindex = getVX();
            break;
            
        case ID_FALL:
            tmap.fall_type = getU2();
            for ( i = 0; i < 3; i++ )
                tmap.falloff.val[ i ] = getF4();
            tmap.falloff.eindex = getVX();
            break;
            
        case ID_OREF:
            tmap.ref_object = getS0();
            break;
            
        case ID_CSYS:
            tmap.coord_sys = getU2();
            break;
            
        default:
            break;
        }
        
        /* error while reading the current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) return 0;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the TMAP subchunk? */
        int fpos = is->tellg();
        if ( fpos == -1 ) break;
        if ( tmapsz + pos <= fpos ) break;
        
//      if ( tmapsz + pos <= is->tellg() )
//          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) return 0;
    };
    
    int g = is->tellg(); 
    flen = g - pos;
    return 1;
}

/*======================================================================
lwGetImageMap()

Read an lwImageMap from a SURF.BLOK in an LWO2 file.
====================================================================== */

lwImageMap *lwReader::lwGetImageMap( int rsz, lwTexture &tex )
{
    unsigned int id;
    unsigned short sz;
    int rlen, pos;
    
    pos = is->tellg();
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) return 0;

    lwImageMap *imap = new lwImageMap;
    if (!imap) return NULL;

    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_TMAP:
            if ( !lwGetTMap( sz, tex.tmap )) return 0;
            break;
            
        case ID_PROJ:
            imap->projection = getU2();
            break;
            
        case ID_VMAP:
            imap->vmap_name = getS0();
            break;
            
        case ID_AXIS:
            imap->axis = getU2();
            break;
            
        case ID_IMAG:
            imap->cindex = getVX();
            break;
            
        case ID_WRAP:
            imap->wrapw_type = getU2();
            imap->wraph_type = getU2();
            break;
            
        case ID_WRPW:
            imap->wrapw.val = getF4();
            imap->wrapw.eindex = getVX();
            break;
            
        case ID_WRPH:
            imap->wraph.val = getF4();
            imap->wraph.eindex = getVX();
            break;
            
        case ID_AAST:
            imap->aas_flags = getU2();
            imap->aa_strength = getF4();
            break;
            
        case ID_PIXB:
            imap->pblend = getU2();
            break;
            
        case ID_STCK:
            imap->stck.val = getF4();
            imap->stck.eindex = getVX();
            break;
            
        case ID_TAMP:
            imap->amplitude.val = getF4();
            imap->amplitude.eindex = getVX();
            break;
            
        default:
            break;
        }
        
        /* error while reading the current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) return 0;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the image map? */
        
        int fpos = is->tellg();
        if ( fpos == -1 ) break;
        if ( rsz + pos <= fpos ) break;

//      if ( rsz + pos <= is->tellg() )
//          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) return 0;
    };
    
    int g = is->tellg(); 
    flen = g - pos;
    return imap;
}


/*
======================================================================
lwGetProcedural()

Read an lwProcedural from a SURF.BLOK in an LWO2 file.
======================================================================
*/
  
lwProcedural *lwReader::lwGetProcedural( int rsz, lwTexture &tex )
{
    unsigned int id;
    unsigned short sz;
    int rlen, pos, fpos;
    
    pos = is->tellg();
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) return 0;

    lwProcedural *proc = new lwProcedural;
    if (!proc) return NULL;
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_TMAP:
            if ( !lwGetTMap( sz, tex.tmap )) return 0;
            break;
            
        case ID_AXIS:
            proc->axis = getU2();
            break;
            
        case ID_VALU:
            proc->value[ 0 ] = getF4();
            if ( sz >= 8 ) proc->value[ 1 ] = getF4();
            if ( sz >= 12 ) proc->value[ 2 ] = getF4();
            break;
            
        case ID_FUNC:
            proc->name = getS0();
            rlen = flen;
            proc->data = getbytes( sz - rlen );
            break;
            
        default:
            break;
        }
        
        /* error while reading the current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) return 0;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the procedural block? */
        
        fpos = is->tellg();
        
        if (fpos == -1)
        {
            flen  = -pos;
            return proc;
        }
        
        if ( rsz + pos <= fpos )
            break;
        
        //      if ( rsz + pos <= is->tellg())
        //          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) return 0;
    };
    
    int g = is->tellg(); 
    flen = g - pos;
    return proc;
}
  
  
/*
======================================================================
lwGetGradient()

Read an lwGradient from a SURF.BLOK in an LWO2 file.
====================================================================== */
  
lwGradient *lwReader::lwGetGradient( int rsz, lwTexture &tex )
{
    unsigned int id;
    unsigned short sz;
    int rlen, pos, i, j, nkeys;
    
    pos = is->tellg();
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) return 0;

    lwGradient *grad = new lwGradient;
    if (!grad) return NULL;

    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_TMAP:
            if ( !lwGetTMap( sz, tex.tmap )) return 0;
            break;
            
        case ID_PNAM:
            grad->paramname = getS0();
            break;
            
        case ID_INAM:
            grad->itemname = getS0();
            break;
            
        case ID_GRST:
            grad->start = getF4();
            break;
            
        case ID_GREN:
            grad->end = getF4();
            break;
            
        case ID_GRPT:
            grad->repeat = getU2();
            break;
            
        case ID_FKEY:
            nkeys = sz / sizeof( lwGradKey );
            grad->key = (lwGradKey *)malloc(nkeys * sizeof(lwGradKey));
            if ( !grad->key ) return 0;
            for ( i = 0; i < nkeys; i++ ) {
                grad->key[ i ].value = getF4();
                for ( j = 0; j < 4; j++ )
                    grad->key[ i ].rgba[ j ] = getF4();
            }
            break;
            
        case ID_IKEY:
            nkeys = sz / 2;
            grad->ikey = (short *)malloc(nkeys * sizeof(short));
            if ( !grad->ikey ) return 0;
            for ( i = 0; i < nkeys; i++ )
                grad->ikey[ i ] = getU2();
            break;
            
        default:
            break;
        }
        
        /* error while reading the current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) return 0;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the gradient? */
        
        int fpos = is->tellg();
        if ( fpos == -1 ) break;
        if ( rsz + pos <= fpos ) break;

//      if ( rsz + pos <= is->tellg() )
//          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) return 0;
    };
    
    int g = is->tellg(); 
    flen = g - pos;
    return grad;
}


/*
======================================================================
lwGetTexture()

Read an lwTexture from a SURF.BLOK in an LWO2 file.
====================================================================== */
  
lwTexture *lwReader::lwGetTexture( int bloksz, unsigned int type )
{
    lwTexture *tex;
    unsigned short sz;
    bool ok;
    
    tex = new lwTexture;
    if ( !tex ) return NULL;
    
    tex->type = type;
    
    sz = getU2();
    if ( !lwGetTHeader( sz, *tex )) {
        delete tex;
        return NULL;
    }
    
    sz = bloksz - sz - 6;
    switch ( type ) {
    case ID_IMAP:
        tex->param.imap = lwGetImageMap( sz, *tex );
        ok = tex->param.imap!=0;
        break;
    case ID_PROC:
        tex->param.proc = lwGetProcedural( sz, *tex );
        ok = tex->param.proc!=0;
        break;
    case ID_GRAD:
        tex->param.grad = lwGetGradient( sz, *tex );
        ok = tex->param.grad!=0;
        break;
    default:
        ok = !is->seekg(sz, ios_base::cur );
    }
    
    if ( !ok )
    {
        delete tex;
        return NULL;
    }
    
    flen = bloksz;
    return tex;
}
  
/*
======================================================================
lwGetShader()

Read a shader record from a SURF.BLOK in an LWO2 file.
====================================================================== */

lwPlugin *lwReader::lwGetShader( int bloksz )
{
    lwPlugin *shdr;
    unsigned int id;
    unsigned short sz;
    int hsz, rlen, pos;
    int g = 0;
    
    shdr = new lwPlugin;
    if ( !shdr ) return NULL;
    
    pos = is->tellg();
    flen = 0;
    hsz = getU2();
    shdr->ord = getS0();
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    while ( hsz > 0 )
    {
        sz += sz & 1;
        hsz -= sz;
        if ( id == ID_ENAB ) {
            shdr->flags = getU2();
            break;
        }
        else {
            is->seekg(sz, ios_base::cur );
            id = getU4();
            sz = getU2();
        }
    }
    
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    for (;;)
    {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_FUNC:
            shdr->name = getS0();
            rlen = flen;
            shdr->data = getbytes( sz - rlen );
            break;
            
        default:
            break;
        }
        
        /* error while reading the current subchunk? */
        
        rlen = flen;
        if ( rlen < 0 || rlen > sz ) goto Fail;
        
        /* skip unread parts of the current subchunk */
        
        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );
        
        /* end of the shader block? */
        
        int fpos = is->tellg();
        if ( fpos == -1 ) break;
        if ( bloksz + pos <= fpos ) break;
        
        
        //      if ( bloksz + pos <= is->tellg() )
        //          break;
        
        /* get the next subchunk header */
        
        flen = 0;
        id = getU4();
        sz = getU2();
        if ( 6 != flen ) goto Fail;
    };
    
    g = is->tellg(); 
    flen = g - pos;
    return shdr;
    
Fail:
    delete shdr;
    return NULL;
}

/*======================================================================
add_clip()

Add a clip to the clip list. Used to store the contents of an RIMG or
TIMG surface subchunk.
======================================================================*/

int lwReader::add_clip( char *s, vclips &clips )
{
    lwClip *clip;
    char *p;

    clip = new lwClip;
    if ( !clip ) return 0;

    if ( p = strstr( s, "(sequence)" ))
    {
        p[ -1 ] = 0;
        clip->type = ID_ISEQ;
        lwClipSeq *seq = new lwClipSeq;
        seq->prefix = s;
        seq->digits = 3;
        clip->source.seq = seq;
    }
    else
    {
        clip->type = ID_STIL;
        lwClipStill *still = new lwClipStill;
        still->name = s;
        clip->source.still = still;
    }

    clips.push_back( clip );
    clip->index = clips.size()-1;

    return clip->index;
}


/*======================================================================
add_tvel()

Add a triple of envelopes to simulate the old texture velocity
parameters.
======================================================================*/

int lwReader::add_tvel( float pos[], float vel[], venvelopes &envelopes )
{
    lwEnvelope *env;
    lwKey *key0, *key1;
    int i;

    for ( i = 0; i < 3; i++ )
    {
        env = new lwEnvelope;
        key0 = new lwKey(0.0f, pos[ i ]);
        key1 = new lwKey(1.0f, pos[ i ] + vel[ i ] * 30.0f); // 30.0f == NTSC related ? 

        key0->shape = key1->shape = ID_LINE;

        env->keys.push_back( key0 );
        env->keys.push_back( key1 );

        env->type = 0x0301 + i;
        env->name = (char *)malloc(11);
        if ( env->name )
        {
            strcpy( env->name, "Position.X" );
            env->name[ 9 ] += i;
        }

        env->behavior[ 0 ] = BEH_LINEAR;
        env->behavior[ 1 ] = BEH_LINEAR;

        envelopes.push_back( env );
        env->index = envelopes.size()-1;
    }
    return env->index - 2;
}


/*======================================================================
get_texture()

Create a new texture for BTEX, CTEX, etc. subchunks.
======================================================================*/

lwTexture *lwReader::get_texture( char *s )
{
    lwTexture *tex = new lwTexture;
    if ( !tex ) return NULL;

    if ( strstr( s, "Image Map" ))
    {
        tex->type = ID_IMAP;
        lwImageMap *imap = new lwImageMap;
        tex->param.imap = imap;
        if ( strstr( s, "Planar" )) imap->projection = 0;
        else if ( strstr( s, "Cylindrical" )) imap->projection = 1;
        else if ( strstr( s, "Spherical" )) imap->projection = 2;
        else if ( strstr( s, "Cubic" )) imap->projection = 3;
        else if ( strstr( s, "Front" )) imap->projection = 4;
        imap->aa_strength = 1.0f;
        imap->amplitude.val = 1.0f;
        free(s);
    }
    else
    {
        tex->type = ID_PROC;
        lwProcedural *proc = new lwProcedural;
        tex->param.proc = proc;
        proc->name = s;
    }
    return tex;
}

lwSurface *lwReader::lwGetLWOBSurface( lwObject *obj )
{
    lwTexture *tex = 0;
    lwPlugin *shdr = 0;
    char *s;
    float v[ 3 ];
    unsigned int flags;
    unsigned short sz;
    int rlen, i;

    long filepos = is->tellg();
    long chunkstart = filepos;
    long chunkend = chunkstart + chunksize;

    /* allocate the Surface structure */

    lwSurface *surf = new lwSurface;
    if ( !surf ) return NULL;

    /* name */

    surf->name = getS0();

    /* process subchunks as they're encountered */
    filepos = is->tellg();
    if (filepos == -1) return surf;
    if ( filepos > chunkend ) return surf; // error: read too much

    while( filepos < chunkend )
    {
        currentchunkid = getU4();
        sz = getU2();
        sz += sz & 1;
        flen = 0;

        switch ( currentchunkid )
        {
        case ID_COLR:
            surf->color.rgb[ 0 ] = getU1() / 255.0f;
            surf->color.rgb[ 1 ] = getU1() / 255.0f;
            surf->color.rgb[ 2 ] = getU1() / 255.0f;
            break;

        case ID_FLAG:
            flags = getU2();
            if ( flags & 4 ) surf->smooth = 1.56207f;
            if ( flags & 8 ) surf->color_hilite.val = 1.0f;
            if ( flags & 16 ) surf->color_filter.val = 1.0f;
            if ( flags & 128 ) surf->dif_sharp.val = 0.5f;
            if ( flags & 256 ) surf->sideflags = 3;
            if ( flags & 512 ) surf->add_trans.val = 1.0f;
            break;

        case ID_LUMI:
            surf->luminosity.val = getI2() / 256.0f;
            break;

        case ID_VLUM:
            surf->luminosity.val = getF4();
            break;

        case ID_DIFF:
            surf->diffuse.val = getI2() / 256.0f;
            break;

        case ID_VDIF:
            surf->diffuse.val = getF4();
            break;

        case ID_SPEC:
            surf->specularity.val = getI2() / 256.0f;
            break;

        case ID_VSPC:
            surf->specularity.val = getF4();
            break;

        case ID_GLOS:
            surf->glossiness.val = (float)log((double)getU2()) / 20.7944f;
            break;

        case ID_SMAN:
            surf->smooth = getF4();
            break;

        case ID_REFL:
            surf->reflection.val.val = getI2() / 256.0f;
            break;

        case ID_RFLT:
            surf->reflection.options = getU2();
            break;

        case ID_RIMG:
            s = getS0();
            surf->reflection.cindex = add_clip( s, obj->clips );
            surf->reflection.options = 3;
            break;

        case ID_RSAN:
            surf->reflection.seam_angle = getF4();
            break;

        case ID_TRAN:
            surf->transparency.val.val = getI2() / 256.0f;
            break;

        case ID_RIND:
            surf->eta.val = getF4();
            break;

        case ID_BTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->bump.textures.push_back( tex );
            break;

        case ID_CTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->color.textures.push_back( tex );
            break;

        case ID_DTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->diffuse.textures.push_back( tex );
            break;

        case ID_LTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->luminosity.textures.push_back( tex );
            break;

        case ID_RTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->reflection.val.textures.push_back( tex );
            break;

        case ID_STEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->specularity.textures.push_back( tex );
            break;

        case ID_TTEX:
            s = getbytes( sz );
            tex = get_texture( s );
            surf->transparency.val.textures.push_back( tex );
            break;

        case ID_TFLG:
            flags = getU2();

            if ( flags & 1 ) i = 0;
            if ( flags & 2 ) i = 1;
            if ( flags & 4 ) i = 2;
            tex->axis = i;
            if ( tex->type == ID_IMAP )
            {
                tex->param.imap->axis = i;
                if ( flags & 32 )
                    tex->param.imap->pblend = 1;
                if ( flags & 64 )
                {
                    tex->param.imap->aa_strength = 1.0f;
                    tex->param.imap->aas_flags = 1;
                }
            }
            if ( tex->type == ID_PROC )
                tex->param.proc->axis = i;

            if ( flags & 8 ) tex->tmap.coord_sys = 1;
            if ( flags & 16 ) tex->negative = 1;
            break;

        case ID_TSIZ:
            for ( i = 0; i < 3; i++ )
                tex->tmap.size.val[ i ] = getF4();
            break;

        case ID_TCTR:
            for ( i = 0; i < 3; i++ )
                tex->tmap.center.val[ i ] = getF4();
            break;

        case ID_TFAL:
            for ( i = 0; i < 3; i++ )
                tex->tmap.falloff.val[ i ] = getF4();
            break;

        case ID_TVEL:
            for ( i = 0; i < 3; i++ )
                v[ i ] = getF4();
            tex->tmap.center.eindex = add_tvel( tex->tmap.center.val, v, obj->envelopes );
            break;

        case ID_TCLR:
            if ( tex->type == ID_PROC )
                for ( i = 0; i < 3; i++ )
                    tex->param.proc->value[ i ] = getU1() / 255.0f;
                break;

        case ID_TVAL:
            if ( tex->type == ID_PROC )
                tex->param.proc->value[ 0 ] = getI2() / 256.0f;
            break;

        case ID_TAMP:
            if ( tex->type == ID_IMAP )
                tex->param.imap->amplitude.val = getF4();
            break;

        case ID_TIMG:
            if ( tex->type == ID_IMAP )
            {
                s = getS0();
                tex->param.imap->cindex = add_clip( s, obj->clips );
            }
            break;

        case ID_TAAS:
            if ( tex->type == ID_IMAP )
            {
                tex->param.imap->aa_strength = getF4();
                tex->param.imap->aas_flags = 1;
            }
            break;

        case ID_TREF:
            tex->tmap.ref_object = getbytes( sz );
            break;

        case ID_TOPC:
            tex->opacity.val = getF4();
            break;

        case ID_TFP0:
            if ( tex->type == ID_IMAP )
                tex->param.imap->wrapw.val = getF4();
            break;

        case ID_TFP1:
            if ( tex->type == ID_IMAP )
                tex->param.imap->wraph.val = getF4();
            break;

        case ID_SHDR:
            shdr = new lwPlugin;
            if ( !shdr ) goto Fail;
            shdr->name = getbytes( sz );
            surf->shaders.push_back( shdr );
            break;

        case ID_SDAT:
            shdr->data = getbytes( sz );
            break;
        default:
            break;
        }

        /* error while reading current subchunk? */

        rlen = flen;
        if ( rlen < 0 || rlen > sz ) goto Fail;

        /* skip unread parts of the current subchunk */

        if ( rlen < sz )
            is->seekg(sz - rlen, ios_base::cur );

        /* end of the SURF chunk? */

        filepos = is->tellg();
        if ( filepos == -1 ) break; // end of file ?
        if ( filepos > chunkend ) // error: read too much
        {
            is->seekg(chunkend, ios_base::beg );
            break;
        }
    }
    return surf;
Fail:
    if ( surf ) delete surf;
    return NULL;
}


/*
======================================================================
lwGetSurface()

Read an lwSurface from an LWO2 file.
====================================================================== */

lwSurface *lwReader::lwGetSurface()
{
    lwSurface *surf;
    lwTexture *tex;
    lwPlugin *shdr;
    unsigned int id, type;
    unsigned short sz;
    unsigned int pos, rlen, fpos;
    
    /* allocate the Surface structure */
    
    surf = new lwSurface;
    if ( !surf ) goto Fail;
    
    /* remember where we started */
    
    flen = 0;
    pos = is->tellg();
    
    /* names */
    
    surf->name = getS0();
    surf->srcname = getS0();
    
    /* first subchunk header */
    
    id = getU4();
    sz = getU2();
    if ( 0 > flen ) goto Fail;
    
    /* process subchunks as they're encountered */
    
    for (;;) {
        sz += sz & 1;
        flen = 0;
        
        switch ( id ) {
        case ID_COLR:
            surf->color.rgb[ 0 ] = getF4();
            surf->color.rgb[ 1 ] = getF4();
            surf->color.rgb[ 2 ] = getF4();
            surf->color.eindex = getVX();
            break;
            
        case ID_LUMI:
            surf->luminosity.val = getF4();
            surf->luminosity.eindex = getVX();
            break;
            
        case ID_DIFF:
            surf->diffuse.val = getF4();
            surf->diffuse.eindex = getVX();
            break;
            
        case ID_SPEC:
            surf->specularity.val = getF4();
            surf->specularity.eindex = getVX();
            break;
            
        case ID_GLOS:
            surf->glossiness.val = getF4();
            surf->glossiness.eindex = getVX();
            break;
            
        case ID_REFL:
            surf->reflection.val.val = getF4();
            surf->reflection.val.eindex = getVX();
            break;
            
        case ID_RFOP:
            surf->reflection.options = getU2();
            break;
            
        case ID_RIMG:
            surf->reflection.cindex = getVX();
            break;
            
        case ID_RSAN:
            surf->reflection.seam_angle = getF4();
            break;
            
        case ID_TRAN:
            surf->transparency.val.val = getF4();
            surf->transparency.val.eindex = getVX();
            break;
            
        case ID_TROP:
            surf->transparency.options = getU2();
            break;
            
        case ID_TIMG:
            surf->transparency.cindex = getVX();
            break;
            
        case ID_RIND:
            surf->eta.val = getF4();
            surf->eta.eindex = getVX();
            break;
            
        case ID_TRNL:
            surf->translucency.val = getF4();
            surf->translucency.eindex = getVX();
            break;
            
        case ID_BUMP:
            surf->bump.val = getF4();
            surf->bump.eindex = getVX();
            break;
            
        case ID_SMAN:
            surf->smooth = getF4();
            break;
            
        case ID_SIDE:
            surf->sideflags = getU2();
            break;
            
        case ID_CLRH:
            surf->color_hilite.val = getF4();
            surf->color_hilite.eindex = getVX();
            break;
            
        case ID_CLRF:
            surf->color_filter.val = getF4();
            surf->color_filter.eindex = getVX();
            break;
            
        case ID_ADTR:
            surf->add_trans.val = getF4();
            surf->add_trans.eindex = getVX();
            break;
            
        case ID_SHRP:
            surf->dif_sharp.val = getF4();
            surf->dif_sharp.eindex = getVX();
            break;
            
        case ID_GVAL:
            surf->glow.val = getF4();
            surf->glow.eindex = getVX();
            break;
            
        case ID_LINE:
            surf->line.enabled = 1;
            if ( sz >= 2 ) surf->line.flags = getU2();
            if ( sz >= 6 ) surf->line.size.val = getF4();
            if ( sz >= 8 ) surf->line.size.eindex = getVX();
            break;
            
        case ID_ALPH:
            surf->alpha_mode = getU2();
            surf->alpha = getF4();
            break;
            
        case ID_AVAL:
            surf->alpha = getF4();
            break;
            
        case ID_BLOK:
            type = getU4();
            
            switch ( type ) {
            case ID_IMAP:
            case ID_PROC:
            case ID_GRAD:
                tex = lwGetTexture( sz - 4, type );
                if ( !tex ) goto Fail;
                if ( !surf->addTexture(tex) ) delete tex;
                flen += 4;
                break;
            case ID_SHDR:
                shdr = lwGetShader( sz - 4 );
                if ( !shdr ) goto Fail;
                surf->shaders.insert(lower_bound(surf->shaders.begin(), surf->shaders.end(), shdr), shdr);
                flen += 4;
                break;
            }
            break;
            
            default:
                break;
    }
    
    /* error while reading current subchunk? */
    
    rlen = flen;
    if ( rlen < 0 || rlen > sz ) goto Fail;
    
    /* skip unread parts of the current subchunk */
    
    if ( rlen < sz )
        is->seekg(sz - rlen, ios_base::cur );
    
    /* end of the SURF chunk? */
    
    fpos = is->tellg();
    if ( fpos == -1 ) break;
    if ( chunksize + pos <= fpos ) break;
    
    /* get the next subchunk header */
    
    flen = 0;
    id = getU4();
    sz = getU2();
    if ( 6 != flen ) goto Fail;
   }
   
   return surf;
   
Fail:
   if ( surf ) delete surf;
   return NULL;
}

/*
======================================================================
lwGetVMap()

  Read an lwVMap from a VMAP or VMAD chunk in an LWO2.
====================================================================== */

lwVMap *lwReader::lwGetVMap( int ptoffset, int poloffset, int perpoly )
{
    char *buf, *bp;
    lwVMap *vmap;
    unsigned int i, j, npts, rlen;
    
    /* read the whole chunk */
    
    flen = 0;
    buf = getbytes( chunksize );
    if ( !buf ) return NULL;
    
    vmap = new lwVMap;
    if ( !vmap ) {
        free(buf);
        return NULL;
    }
    
    /* initialize the vmap */
    
    vmap->perpoly = perpoly;
    
    bp = buf;
    flen = 0;
    vmap->type = sgetU4( &bp );
    vmap->dim  = sgetU2( &bp );
    vmap->name = sgetS0( &bp );
    rlen = flen;
    
    /* count the vmap records */
    
    npts = 0;
    while ( bp < buf + chunksize ) {
        i = sgetVX( &bp );
        if ( perpoly )
            i = sgetVX( &bp );
        bp += vmap->dim * sizeof( float );
        ++npts;
    }
    
    /* allocate the vmap */
    
    vmap->nverts = npts;
    vmap->vindex = (int *)malloc(npts * sizeof(int));
    if ( !vmap->vindex ) goto Fail;
    if ( perpoly )
    {
        vmap->pindex = (int *)malloc(npts * sizeof(int));
        if ( !vmap->pindex ) goto Fail;
    }
    
    if ( vmap->dim > 0 )
    {
        vmap->val = (float **)malloc(npts * sizeof(float *));
        if ( !vmap->val ) goto Fail;
        for ( i = 0; i < npts; i++ )
            vmap->val[ i ] = (float *)malloc(vmap->dim * sizeof( float ));
    }
    
    /* fill in the vmap values */
    
    bp = buf + rlen;
    for ( i = 0; i < npts; i++ ) {
        vmap->vindex[ i ] = sgetVX( &bp );
        if ( perpoly )
            vmap->pindex[ i ] = sgetVX( &bp );
        for ( j = 0; j < vmap->dim; j++ )
            vmap->val[ i ][ j ] = sgetF4( &bp );
    }
    
    free(buf);
    return vmap;
    
Fail:
    free(buf);
    delete vmap;
    return NULL;
}
