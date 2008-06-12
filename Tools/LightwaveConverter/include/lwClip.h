#ifndef _LWCLIP_H_
#define _LWCLIP_H_

class lwClipStill
{
public:
	lwClipStill()
	{
		name = 0;
	}
	~lwClipStill()
	{
		if (name) free(name);
	}
	char *name;
};

class lwClipSeq
{
public:
	lwClipSeq()
	{
		prefix = 0;
		suffix = 0;
	}
	~lwClipSeq()
	{
			if (prefix) free(prefix);
			if (suffix) free(suffix);
	}
	char          *prefix;              /* filename before sequence digits */
	char          *suffix;              /* after digits, e.g. extensions */
	int            digits;
	int            flags;
	int            offset;
	int            start;
	int            end;
};

class lwClipAnim
{
public:
	lwClipAnim()
	{
		name = 0;
		server = 0;
		data = 0;
	}
	~lwClipAnim()
	{
		if (name) free(name);
		if (server) free(server);
		if (data) free(data);
	}
	char          *name;
	char          *server;              /* anim loader plug-in */
	void          *data;
};

class lwClipXRef
{
public:
	lwClipXRef()
	{
		string = 0;
		clip = 0;
	}
	~lwClipXRef()
	{
		if (string) free(string);
	}

	char          *string;
	int            index;
	class lwClip  *clip;
};

class lwClipCycle {
public:
	lwClipCycle()
	{
		name = 0;
	}
	~lwClipCycle()
	{
		if (name) free(name);
	}
	char          *name;
	int            lo;
	int            hi;
};

class lwClip {
public:
	lwClip()
	{
		source.still = 0;
		contrast.val = 1.0f;
		brightness.val = 1.0f;
		saturation.val = 1.0f;
		gamma.val = 1.0f;
	}
	
	~lwClip()
	{
		unsigned int i;
		for (i=0; i < ifilters.size(); delete ifilters[i++]);
		for (i=0; i < pfilters.size(); delete pfilters[i++]);
		
		if (source.still)
		{
			switch (type)
			{
			case ID_STIL:
				delete source.still;
				break;
			case ID_ISEQ:
				delete source.seq;
				break;
			case ID_ANIM:
				delete source.anim;
				break;
			case ID_XREF:
				delete source.xref;
				break;
			case ID_STCC:
				delete source.cycle;
				break;
			default:
				;
			}
		}
	}
	int               index;
	unsigned int      type;                /* ID_STIL, ID_ISEQ, etc. */
	union {
		lwClipStill  *still;
		lwClipSeq    *seq;
		lwClipAnim   *anim;
		lwClipXRef   *xref;
		lwClipCycle  *cycle;
	}                 source;
	float             start_time;
	float             duration;
	float             frame_rate;
	lwEParam          contrast;
	lwEParam          brightness;
	lwEParam          saturation;
	lwEParam          hue;
	lwEParam          gamma;
	int               negative;
	vplugins ifilters;             /* linked list of image filters */
	vplugins pfilters;             /* linked list of pixel filters */
};

typedef vector<lwClip*> vclips;

#endif // _LWCLIP_H_

