#ifndef _LWOBJECT_H_
#define _LWOBJECT_H_

#include "lwo.h"
#include "lwEnvelope.h"
#include "lwClip.h"
#include "lwLayer.h"

class lwObject
{
public:
	lwObject()
	{
		tagsoffset = 0;
	}

	~lwObject()
	{
		unsigned int i;

		for (i=0; i < layers.size(); delete layers[i++]);
		for (i=0; i < envelopes.size(); delete envelopes[i++]);
		for (i=0; i < clips.size(); delete clips[i++]);
		for (i=0; i < surfaces.size(); i++)
		{
			lwSurface *s = surfaces[i];
			if(s)
			{
				for (unsigned int j = i+1; j < surfaces.size(); j++)
					if (s == surfaces[j]) surfaces[j] = 0;
				delete s;
			}
		}
		for (i=0; i < tags.size(); free(tags[i++]));
	}

	lwClip *lwFindClip( int index )
	{
		for (unsigned int i = 0; i < clips.size(); i++)
			if (clips[i]->index == index)
				return clips[i];
		return 0;
	}

	lwEnvelope *lwFindEnvelope( int index )
	{
		for (unsigned int i = 0; i < envelopes.size(); i++)
			if (envelopes[i]->index == index)
				return envelopes[i];
		return 0;
	}

	vlayers    layers;     /* linked list of layers */
	venvelopes envelopes;  /* linked list of envelopes */
	vclips     clips;      /* linked list of clips */
	vsurfaces  surfaces;   /* linked list of surfaces */
	int        tagsoffset;
	vtags      tags;       /* array of strings */
};

typedef vector<lwObject*> vobjects;

#endif // _LWOBJECT_H_

