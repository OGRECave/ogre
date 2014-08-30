/*

Lightwave Object Loader
(LWOB, LWLO, LWO2)

converted by Dennis Verbeek (dennis.verbeek@chello.nl)

These files were originally coded by Ernie Wright and were included in the 
Lightwave SDK.
*/

#ifndef _LWO_H_
#define _LWO_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Point.h"

//using namespace std;

/* chunk and subchunk IDs */

#define LWID_(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#define ID_FORM LWID_('F','O','R','M')
#define ID_LWO2 LWID_('L','W','O','2')
#define ID_LWOB LWID_('L','W','O','B')
#define ID_LWLO LWID_('L','W','L','O')

/* top-level chunks */
#define ID_LAYR LWID_('L','A','Y','R')
#define ID_TAGS LWID_('T','A','G','S')
#define ID_PNTS LWID_('P','N','T','S')
#define ID_BBOX LWID_('B','B','O','X')
#define ID_VMAP LWID_('V','M','A','P')
#define ID_VMAD LWID_('V','M','A','D')
#define ID_POLS LWID_('P','O','L','S')
#define ID_PTAG LWID_('P','T','A','G')
#define ID_ENVL LWID_('E','N','V','L')
#define ID_CLIP LWID_('C','L','I','P')
#define ID_SURF LWID_('S','U','R','F')
#define ID_DESC LWID_('D','E','S','C')
#define ID_TEXT LWID_('T','E','X','T')
#define ID_ICON LWID_('I','C','O','N')

/* polygon types */
#define ID_FACE LWID_('F','A','C','E')
#define ID_CURV LWID_('C','U','R','V')
#define ID_PTCH LWID_('P','T','C','H')
#define ID_MBAL LWID_('M','B','A','L')
#define ID_BONE LWID_('B','O','N','E')

/* polygon tags */
#define ID_SURF LWID_('S','U','R','F')
#define ID_PART LWID_('P','A','R','T')
#define ID_SMGP LWID_('S','M','G','P')

/* envelopes */
#define ID_PRE  LWID_('P','R','E',' ')
#define ID_POST LWID_('P','O','S','T')
#define ID_KEY  LWID_('K','E','Y',' ')
#define ID_SPAN LWID_('S','P','A','N')
#define ID_TCB  LWID_('T','C','B',' ')
#define ID_HERM LWID_('H','E','R','M')
#define ID_BEZI LWID_('B','E','Z','I')
#define ID_BEZ2 LWID_('B','E','Z','2')
#define ID_LINE LWID_('L','I','N','E')
#define ID_STEP LWID_('S','T','E','P')

/* clips */
#define ID_STIL LWID_('S','T','I','L')
#define ID_ISEQ LWID_('I','S','E','Q')
#define ID_ANIM LWID_('A','N','I','M')
#define ID_XREF LWID_('X','R','E','F')
#define ID_STCC LWID_('S','T','C','C')
#define ID_TIME LWID_('T','I','M','E')
#define ID_CONT LWID_('C','O','N','T')
#define ID_BRIT LWID_('B','R','I','T')
#define ID_SATR LWID_('S','A','T','R')
#define ID_HUE  LWID_('H','U','E',' ')
#define ID_GAMM LWID_('G','A','M','M')
#define ID_NEGA LWID_('N','E','G','A')
#define ID_IFLT LWID_('I','F','L','T')
#define ID_PFLT LWID_('P','F','L','T')

/* surfaces */
#define ID_COLR LWID_('C','O','L','R')
#define ID_LUMI LWID_('L','U','M','I')
#define ID_DIFF LWID_('D','I','F','F')
#define ID_SPEC LWID_('S','P','E','C')
#define ID_GLOS LWID_('G','L','O','S')
#define ID_REFL LWID_('R','E','F','L')
#define ID_RFOP LWID_('R','F','O','P')
#define ID_RIMG LWID_('R','I','M','G')
#define ID_RSAN LWID_('R','S','A','N')
#define ID_TRAN LWID_('T','R','A','N')
#define ID_TROP LWID_('T','R','O','P')
#define ID_TIMG LWID_('T','I','M','G')
#define ID_RIND LWID_('R','I','N','D')
#define ID_TRNL LWID_('T','R','N','L')
#define ID_BUMP LWID_('B','U','M','P')
#define ID_SMAN LWID_('S','M','A','N')
#define ID_SIDE LWID_('S','I','D','E')
#define ID_CLRH LWID_('C','L','R','H')
#define ID_CLRF LWID_('C','L','R','F')
#define ID_ADTR LWID_('A','D','T','R')
#define ID_SHRP LWID_('S','H','R','P')
#define ID_LINE LWID_('L','I','N','E')
#define ID_LSIZ LWID_('L','S','I','Z')
#define ID_ALPH LWID_('A','L','P','H')
#define ID_AVAL LWID_('A','V','A','L')
#define ID_GVAL LWID_('G','V','A','L')
#define ID_BLOK LWID_('B','L','O','K')

/* texture layer */
#define ID_TYPE LWID_('T','Y','P','E')
#define ID_CHAN LWID_('C','H','A','N')
#define ID_NAME LWID_('N','A','M','E')
#define ID_ENAB LWID_('E','N','A','B')
#define ID_OPAC LWID_('O','P','A','C')
#define ID_FLAG LWID_('F','L','A','G')
#define ID_PROJ LWID_('P','R','O','J')
#define ID_STCK LWID_('S','T','C','K')
#define ID_TAMP LWID_('T','A','M','P')

/* texture coordinates */
#define ID_TMAP LWID_('T','M','A','P')
#define ID_AXIS LWID_('A','X','I','S')
#define ID_CNTR LWID_('C','N','T','R')
#define ID_SIZE LWID_('S','I','Z','E')
#define ID_ROTA LWID_('R','O','T','A')
#define ID_OREF LWID_('O','R','E','F')
#define ID_FALL LWID_('F','A','L','L')
#define ID_CSYS LWID_('C','S','Y','S')

/* image map */
#define ID_IMAP LWID_('I','M','A','P')
#define ID_IMAG LWID_('I','M','A','G')
#define ID_WRAP LWID_('W','R','A','P')
#define ID_WRPW LWID_('W','R','P','W')
#define ID_WRPH LWID_('W','R','P','H')
#define ID_VMAP LWID_('V','M','A','P')
#define ID_AAST LWID_('A','A','S','T')
#define ID_PIXB LWID_('P','I','X','B')

/* procedural */
#define ID_PROC LWID_('P','R','O','C')
#define ID_COLR LWID_('C','O','L','R')
#define ID_VALU LWID_('V','A','L','U')
#define ID_FUNC LWID_('F','U','N','C')
#define ID_FTPS LWID_('F','T','P','S')
#define ID_ITPS LWID_('I','T','P','S')
#define ID_ETPS LWID_('E','T','P','S')

/* gradient */
#define ID_GRAD LWID_('G','R','A','D')
#define ID_GRST LWID_('G','R','S','T')
#define ID_GREN LWID_('G','R','E','N')
#define ID_PNAM LWID_('P','N','A','M')
#define ID_INAM LWID_('I','N','A','M')
#define ID_GRPT LWID_('G','R','P','T')
#define ID_FKEY LWID_('F','K','E','Y')
#define ID_IKEY LWID_('I','K','E','Y')

/* shader */
#define ID_SHDR LWID_('S','H','D','R')
#define ID_DATA LWID_('D','A','T','A')

/* IDs specific to LWOB */
#define ID_SRFS LWID_('S','R','F','S')
#define ID_FLAG LWID_('F','L','A','G')
#define ID_VLUM LWID_('V','L','U','M')
#define ID_VDIF LWID_('V','D','I','F')
#define ID_VSPC LWID_('V','S','P','C')
#define ID_RFLT LWID_('R','F','L','T')
#define ID_BTEX LWID_('B','T','E','X')
#define ID_CTEX LWID_('C','T','E','X')
#define ID_DTEX LWID_('D','T','E','X')
#define ID_LTEX LWID_('L','T','E','X')
#define ID_RTEX LWID_('R','T','E','X')
#define ID_STEX LWID_('S','T','E','X')
#define ID_TTEX LWID_('T','T','E','X')
#define ID_TFLG LWID_('T','F','L','G')
#define ID_TSIZ LWID_('T','S','I','Z')
#define ID_TCTR LWID_('T','C','T','R')
#define ID_TFAL LWID_('T','F','A','L')
#define ID_TVEL LWID_('T','V','E','L')
#define ID_TCLR LWID_('T','C','L','R')
#define ID_TVAL LWID_('T','V','A','L')
#define ID_TAMP LWID_('T','A','M','P')
#define ID_TIMG LWID_('T','I','M','G')
#define ID_TAAS LWID_('T','A','A','S')
#define ID_TREF LWID_('T','R','E','F')
#define ID_TOPC LWID_('T','O','P','C')
#define ID_SDAT LWID_('S','D','A','T')
#define ID_TFP0 LWID_('T','F','P','0')
#define ID_TFP1 LWID_('T','F','P','1')

/* Unknown tags */

#define ID_TFP2 LWID_('T','F','P','2')
#define ID_TFP3 LWID_('T','F','P','3')
#define ID_SHCP LWID_('S','H','C','P')
#define ID_CRVS LWID_('C','R','V','S')

/* plug-in reference */

class lwPlugin
{
public:
    lwPlugin()
    {
        ord = 0;
        name = 0;
        data = 0;
    }

    ~lwPlugin()
    {
        if (ord) free(ord);
        if (name) free(name);
        if (data) free(data);
    }

    char          *ord;
    char          *name;
    int            flags;
    void          *data;
};

typedef vector<lwPlugin*> vplugins;

inline bool operator < (const lwPlugin &p1, const lwPlugin &p2 )
{
    return strcmp( p1.ord, p2.ord ) < 0;
}

class lwEParam
{
public:
    lwEParam()
    {
        val = 0.0f;
        eindex = 0;
    }
    float val;
    int   eindex;
};

class lwVParam
{
public:
    lwVParam()
    {
        val[0] = val[1] = val[2] = 0.0f;
        eindex = 0;
    }
    float val[ 3 ];
    int   eindex;
};


/* clips */

/* textures */

class lwTMap {
public:
    lwTMap()
    {
        ref_object = 0;
    }

    ~lwTMap()
    {
        if (ref_object) free(ref_object);
    }
    lwVParam       size;
    lwVParam       center;
    lwVParam       rotate;
    lwVParam       falloff;
    int            fall_type;
    char          *ref_object;
    int            coord_sys;
};

class lwImageMap {
public:
    lwImageMap()
    {
        vmap_name = 0;
    }

    ~lwImageMap()
    {
        if (vmap_name) free(vmap_name);
    }

    int            cindex;
    int            projection;
    char          *vmap_name;
    int            axis;
    int            wrapw_type;
    int            wraph_type;
    lwEParam       wrapw;
    lwEParam       wraph;
    float          aa_strength;
    int            aas_flags;
    int            pblend;
    lwEParam       stck;
    lwEParam       amplitude;
};

#define PROJ_PLANAR       0
#define PROJ_CYLINDRICAL  1
#define PROJ_SPHERICAL    2
#define PROJ_CUBIC        3
#define PROJ_FRONT        4

#define WRAP_NONE    0
#define WRAP_EDGE    1
#define WRAP_REPEAT  2
#define WRAP_MIRROR  3

class lwProcedural {
public:
    lwProcedural()
    {
        name = 0;
        data = 0;
    }

    ~lwProcedural()
    {
        if (name) free(name);
        if (data) free(data);
    }

    int            axis;
    float          value[ 3 ];
    char          *name;
    void          *data;
};

class lwGradKey {
public:
    float          value;
    float          rgba[ 4 ];
};

class lwGradient {
public:
    lwGradient()
    {
        paramname = 0;
        itemname = 0;
        key = 0;
        ikey = 0;
    }

    ~lwGradient()
    {
        if (paramname) free(paramname);
        if (itemname) free(itemname);
        if (key) free(key);
        if (ikey) free(ikey);
    }

    char          *paramname;
    char          *itemname;
    float          start;
    float          end;
    int            repeat;
    lwGradKey     *key;                 /* array of gradient keys */
    short         *ikey;                /* array of interpolation codes */
} ;

class lwTexture
{
public:
    lwTexture()
    {
        ord = 0;
        param.imap = 0;
        
        tmap.size.val[ 0 ] =
            tmap.size.val[ 1 ] =
            tmap.size.val[ 2 ] = 1.0f;
        opacity.val = 1.0f;
        enabled = 1;        
    }

    ~lwTexture()
    {
        if (ord) free(ord);
        if(param.imap)
        {
            switch (type)
            {
            case ID_IMAP:
                delete param.imap;
                break;
            case ID_PROC:
                delete param.proc;
                break;
            case ID_GRAD:
                delete param.grad;
                break;
            default:
                ;
            }
        }
    }
    char          *ord;
    unsigned int   type;
    unsigned int   chan;
    lwEParam       opacity;
    short          opac_type;
    short          enabled;
    short          negative;
    short          axis;
    union
    {
        lwImageMap     *imap;
        lwProcedural   *proc;
        lwGradient     *grad;
    }              param;
    lwTMap         tmap;
};

typedef vector<lwTexture*> vtextures;

/* values that can be textured */

class lwTParam
{
public:
    lwTParam()
    {
        val = 0;
        eindex = 0;
    }

    ~lwTParam()
    {
        for (unsigned int i=0; i < textures.size(); delete textures[i++]);
    }

    void addTexture( lwTexture *tex )
    {
        textures.insert(lower_bound(textures.begin(), textures.end(), tex), tex);
    }
    
    float     val;
    int       eindex;
    vtextures textures;    /* linked list of texture layers */
};

class lwCParam
{
public:
    lwCParam()
    {
        rgb[0] = 0.78431f;
        rgb[1] = 0.78431f;
        rgb[2] = 0.78431f;
        eindex = 0;
    }

    ~lwCParam()
    {
        for (unsigned int i=0; i < textures.size(); delete textures[i++]);
    }

    void addTexture( lwTexture *tex )
    {
        textures.insert(lower_bound(textures.begin(), textures.end(), tex), tex);
    }

    float     rgb[ 3 ];
    int       eindex;
    vtextures textures;    /* linked list of texture layers */
};


/* surfaces */

class lwGlow
{
public:
    short          enabled;
    short          type;
    lwEParam       intensity;
    lwEParam       size;
};

class lwRMap
{
public:
    lwRMap()
    {
        options = 0;
        cindex = 0;
        seam_angle = 0.0f;
    }
    lwTParam       val;
    int            options;
    int            cindex;
    float          seam_angle;
};

class lwLine
{
public:
    short          enabled;
    unsigned short flags;
    lwEParam       size;
};

class lwSurface
{
public:
    lwSurface()
    {
        name = 0;
        srcname = 0;
        diffuse.val    = 1.0f;
        glossiness.val = 0.4f;
        bump.val       = 1.0f;
        eta.val        = 1.0f;
        sideflags      = 1;
    }
    
    ~lwSurface()
    {
        if (name) free(name);
        if (srcname) free(srcname);
        for (unsigned int i=0; i < shaders.size(); delete shaders[i++]);
    }
    
    int addTexture( lwTexture *tex )
    {
        switch ( tex->chan )
        {
        case ID_COLR:
            color.addTexture(tex);
            break;
        case ID_LUMI:
            luminosity.addTexture(tex);
            break;
        case ID_DIFF:
            diffuse.addTexture(tex);
            break;
        case ID_SPEC:
            specularity.addTexture(tex);
            break;
        case ID_GLOS:
            glossiness.addTexture(tex);
            break;
        case ID_REFL:
            reflection.val.addTexture(tex);
            break;
        case ID_TRAN:
            transparency.val.addTexture(tex);
            break;
        case ID_RIND:
            eta.addTexture(tex);
            break;
        case ID_TRNL:
            translucency.addTexture(tex);
            break;
        case ID_BUMP:
            bump.addTexture(tex);
            break;
        default:
            return 0;
        }
        return 1;
    }
    
    static lwSurface *lwDefaultSurface( void )
    {
        return new lwSurface;
    }

    char *setname(const char *newname)
    {
        unsigned int slength = strlen(newname);

        if (name && slength > strlen(name))
        {
            free(name);
            name = 0;           
        }
        if (!name) name = (char *)malloc(slength+1);

        return strcpy(name, newname);
    }
    
    char             *name;
    char             *srcname;
    lwCParam          color;
    lwTParam          luminosity;
    lwTParam          diffuse;
    lwTParam          specularity;
    lwTParam          glossiness;
    lwRMap            reflection;
    lwRMap            transparency;
    lwTParam          eta;
    lwTParam          translucency;
    lwTParam          bump;
    float             smooth;
    int               sideflags;
    float             alpha;
    int               alpha_mode;
    lwEParam          color_hilite;
    lwEParam          color_filter;
    lwEParam          add_trans;
    lwEParam          dif_sharp;
    lwEParam          glow;
    lwLine            line;
    vplugins shaders;              /* linked list of shaders */
};

typedef vector<lwSurface*> vsurfaces;

/* vertex maps */

class lwVMap
{
public:
    lwVMap()
    {
        name = 0;
        vindex = 0;
        pindex = 0;
        val = 0;
    }

    ~lwVMap()
    {
        if (name) free(name);
        if (vindex) free(vindex);
        if (pindex) free(pindex);
        
        if (val) 
        {
            for (unsigned int i = 0; i < nverts; free(val[i++]));
            free (val);
        }
    }
    
    char          *name;
    unsigned int   type;
    unsigned int   dim;
    unsigned int   nverts;
    int            perpoly;
    int           *vindex;              /* array of point indexes */
    int           *pindex;              /* array of polygon indexes */
    float        **val;
};

typedef vector<lwVMap*> vvmaps;

class lwVMapPt
{
    lwVMapPt();
public:
    lwVMapPt(lwVMap *nvmap, int nindex) : vmap(nvmap), index(nindex) {};

    lwVMap        *vmap;
    int            index;               /* vindex or pindex element */
};

typedef vector<lwVMapPt> vvmapptrs;

/* points and polygons */
class lwPolygon;

typedef vector<lwPolygon*> vpolygons;

class lwPoint: public Point3
{
    lwPoint();
public:
    lwPoint(float r, float s, float t) : Point3(r, s, t) {};

    vvmapptrs      vmaps;               /* array of vmap references */
    vpolygons      polygons;            /* array of polygon indexes */
    unsigned short index;
};

typedef vector<lwPoint*> vpoints;

class lwVertex
{
    lwVertex();

public:
    lwVertex(int nindex) : index(nindex) {}
    
    lwVertex(const lwVertex &v)
    {
        index = v.index;
        point = v.point;
        normal = v.normal;
        vmaps = v.vmaps;
    }
    
    int        index;      /* index into the point array */
    lwPoint   *point;
    Vector3    normal;
    vvmapptrs  vmaps;      /* array of vmap references */
};

typedef vector<lwVertex*> vvertices;

typedef vector<char *> vtags;

inline bool operator < (const lwTexture &t1, const lwTexture &t2 )
{
    return strcmp( t1.ord, t2.ord ) < 0;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
void revbytes( void *bp, int elsize, int elcount );
#else
#define revbytes( b, s, c )
#endif

#endif // _LWO_H_

