
#ifndef _MqMessages_H_
#define _MqMessages_H_

#include <vector>
#include <assert.h>

namespace Demo
{
namespace Mq
{
    enum MessageId
    {
        //Graphics  -> Logic
        SDL_EVENT,
        //Graphics <-> Logic
        SDL_EVENT_BUFFER_ID_USED,

        NUM_MESSAGE_IDS
    };

    class DestructibleData
    {
    public:
        virtual ~DestructibleData() {}
    };

    enum DataType
    {
        IsInvalidData,
        IsPodData,
        IsVarData,
        IsDestrData
    };

    union UnifiedData
    {
        bool		bData;
        float		fData;
        int			iData;
        void*		data;

        UnifiedData()
        {
        }
        UnifiedData( void *d ) : data(d)
        {
        }
        UnifiedData( float f ) : fData(f)
        {
        }
        UnifiedData( int i ) : iData(i)
        {
        }
        UnifiedData( bool b ) : bData(b)
        {
        }
    };

    struct SendData
    {
        DataType    dataType;
        UnifiedData udata;

        SendData() : dataType( IsInvalidData ) {}
        SendData( DataType _dataType )      : dataType( _dataType )				{}
        SendData( DestructibleData *d )		: dataType( IsDestrData ), udata(d)	{}
        SendData( float f )					: dataType( IsVarData ), udata(f)	{}
        SendData( int i )					: dataType( IsVarData ), udata(i)	{}
        SendData( bool b )					: dataType( IsVarData ), udata(b)	{}
        //Comment this one to get warning on pointers, which
        //probably should be converted to DestructibleData
        SendData( void *d )					: dataType( IsPodData ), udata(d)	{}
    };

    struct Message
    {
        MessageId	mMessageId;
        SendData    mData;
        bool		mDeleteData; //True to delete data pointer after sending

        Message( MessageId messageId, const SendData &data, bool bDelete ) :
            mMessageId( messageId ),
            mData( data ),
            mDeleteData( bDelete )
        {
            assert( ((mDeleteData && mData.dataType == IsDestrData) || !mDeleteData) &&
                    "Only messages deriving from DestructibleData can be destroyed!" );
        }
    };

    typedef std::vector<Message>	MessageVec;
}
}

#endif
