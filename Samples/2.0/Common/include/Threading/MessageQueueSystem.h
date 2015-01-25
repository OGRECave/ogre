
#ifndef _MessageQueueSystem_H_
#define _MessageQueueSystem_H_

#include "Threading/OgreLightweightMutex.h"
#include "MqMessages.h"

#include <map>

namespace Demo
{
    class MessageQueueSystem
    {
        Ogre::LightweightMutex  mMessageQueueMutex;

        typedef std::map<MessageQueueSystem*, Mq::MessageVec> PendingMessageMap;

        PendingMessageMap   mPendingOutgoingMessages;

        Mq::MessageVec      mIncomingMessages[2];

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
            The message itself
        */
        void queueSendMessage( MessageQueueSystem *dstSystem, const Mq::Message &msg )
        {
            mPendingOutgoingMessages[dstSystem].push_back( msg );
        }

        /// Sends all the messages queue via @see queueSendMessage();
        /// Must be called from the thread that owns 'this'
        void flushQueuedMessages(void)
        {
            PendingMessageMap::const_iterator itMap = mPendingOutgoingMessages.begin();
            PendingMessageMap::const_iterator enMap = mPendingOutgoingMessages.end();

            while( itMap != enMap )
            {
                MessageQueueSystem *dstSystem = itMap->first;

                dstSystem->mMessageQueueMutex.lock();

                dstSystem->mIncomingMessages[0].insert(
                            dstSystem->mIncomingMessages[0].end(),
                            itMap->second.begin(),
                            itMap->second.end() );

                dstSystem->mMessageQueueMutex.unlock();

                ++itMap;
            }

            mPendingOutgoingMessages.clear();
        }

        /// Sends a message to 'this' base system immediately. Use it only for
        /// time critical messages or if the sender thread doesn't own its own
        /// MessageQueueSystem class.
        /// Abusing this function can degrade performance as it would perform
        /// frequent locking.
        void receiveMessageImmediately( const Mq::Message &msg )
        {
            mMessageQueueMutex.lock();
            mIncomingMessages[0].push_back( msg );
            mMessageQueueMutex.unlock();
        }

    protected:
        /// Processes all incoming messages sent from other threads.
        /// Should be called from the thread that owns 'this'
        void processIncomingMessages(void)
        {
            mMessageQueueMutex.lock();
            mIncomingMessages[0].swap( mIncomingMessages[1] );
            mMessageQueueMutex.unlock();

            Mq::MessageVec::const_iterator itor = mIncomingMessages[1].begin();
            Mq::MessageVec::const_iterator end  = mIncomingMessages[1].end();

            while( itor != end )
            {
                processIncomingMessage( itor->mMessageId, itor->mData );

                if( itor->mDeleteData )
                {
                    delete reinterpret_cast<Mq::DestructibleData*>( itor->mData.udata.data );
                }

                ++itor;
            }

            mIncomingMessages[1].clear();
        }

        /// Derived classes must implement this function to process the incoming message
        virtual void processIncomingMessage( Mq::MessageId messageId, Mq::SendData data ) = 0;
    };
}

#endif
