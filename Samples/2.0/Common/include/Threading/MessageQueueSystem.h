
#ifndef _Mq_MessageQueueSystem_H_
#define _Mq_MessageQueueSystem_H_

#include "Threading/OgreLightweightMutex.h"
#include "OgreCommon.h"
#include "OgreFastArray.h"
#include "MqMessages.h"

#include <map>

namespace Demo
{
namespace Mq
{
    class MessageQueueSystem
    {
        static const size_t cSizeOfHeader;

        typedef Ogre::FastArray<unsigned char> MessageArray;
        typedef std::map<MessageQueueSystem*, MessageArray> PendingMessageMap;

        Ogre::LightweightMutex  mMessageQueueMutex;

        PendingMessageMap   mPendingOutgoingMessages;
        MessageArray        mIncomingMessages[2];

        template <typename T> static void storeMessageToQueue( MessageArray &queue,
                                                               Mq::MessageId messageId, const T &msg )
        {
            //Save the current offset.
            const size_t startOffset = queue.size();

            //Enlarge the queue. Preserve alignment.
            const size_t totalSize = Ogre::alignToNextMultiple( cSizeOfHeader + sizeof(T),
                                                                sizeof(size_t) );
            queue.resize( queue.size() + totalSize );

            //Write the header: the Size and the MessageId
            *reinterpret_cast<Ogre::uint32*>( queue.begin() + startOffset ) = totalSize;
            *reinterpret_cast<Ogre::uint32*>( queue.begin() + startOffset +
                                              sizeof(Ogre::uint32) )        = messageId;

            //Write the actual message.
            T *dstPtr = reinterpret_cast<T*>( queue.begin() + startOffset + cSizeOfHeader );
            memcpy( dstPtr, &msg, sizeof( T ) );
        }

    public:
        virtual ~MessageQueueSystem()
        {
        }

        /** Queues message 'msg' to be sent to a destination MessageQueueSystem.
            This function *must* be called from the thread that owns 'this'
            The 'dstSystem' may live in any other thread.
        @remarks
            The message is not instantely delivered. It will be sent when
            flushQueuedMessages gets called.
        @param dstSystem
            The MessageQueueSystem we want to send a message to.
        @param msg
            The message itself. Structure must be POD.
        */
        template <typename T>
        void queueSendMessage( MessageQueueSystem *dstSystem, Mq::MessageId messageId, const T &msg )
        {
            storeMessageToQueue( mPendingOutgoingMessages[dstSystem], messageId, msg );
        }

        /// Sends all the messages queued via see queueSendMessage();
        /// Must be called from the thread that owns 'this'
        void flushQueuedMessages(void)
        {
            PendingMessageMap::iterator itMap = mPendingOutgoingMessages.begin();
            PendingMessageMap::iterator enMap = mPendingOutgoingMessages.end();

            while( itMap != enMap )
            {
                MessageQueueSystem *dstSystem = itMap->first;

                dstSystem->mMessageQueueMutex.lock();

                dstSystem->mIncomingMessages[0].appendPOD(
                            itMap->second.begin(),
                            itMap->second.end() );

                dstSystem->mMessageQueueMutex.unlock();

                itMap->second.clear();

                ++itMap;
            }

            //mPendingOutgoingMessages.clear();
        }

        /// Sends a message to 'this' base system immediately. Use it only for
        /// time critical messages or if the sender thread doesn't own its own
        /// MessageQueueSystem class.
        /// Abusing this function can degrade performance as it would perform
        /// frequent locking. See queueSendMessage
        template <typename T>
        void receiveMessageImmediately( Mq::MessageId messageId, const T &msg )
        {
            mMessageQueueMutex.lock();
            storeMessageToQueue( mIncomingMessages[0], messageId, msg );
            mMessageQueueMutex.unlock();
        }

    protected:
        /// Processes all incoming messages received from other threads.
        /// Should be called from the thread that owns 'this'
        void processIncomingMessages(void)
        {
            mMessageQueueMutex.lock();
            mIncomingMessages[0].swap( mIncomingMessages[1] );
            mMessageQueueMutex.unlock();

            MessageArray::const_iterator itor = mIncomingMessages[1].begin();
            MessageArray::const_iterator end  = mIncomingMessages[1].end();

            while( itor != end )
            {
                Ogre::uint32 totalSize = *reinterpret_cast<const Ogre::uint32*>( itor );
                Ogre::uint32 messageId = *reinterpret_cast<const Ogre::uint32*>( itor +
                                                                                 sizeof(Ogre::uint32) );

                assert( itor + totalSize <= end && "MessageQueue corrupted!" );
                assert( messageId <= Mq::NUM_MESSAGE_IDS &&
                        "MessageQueue corrupted or invalid message!" );

                const void *data = itor + cSizeOfHeader;
                processIncomingMessage( static_cast<Mq::MessageId>( messageId ), data );
                itor += totalSize;
            }

            mIncomingMessages[1].clear();
        }

        /// Derived classes must implement this function to process the incoming message
        virtual void processIncomingMessage( Mq::MessageId messageId, const void *data ) = 0;
    };
}
}

#endif
