
#include "StdAfx.h"

#include "Client.h"

//Network access.
#include "crc.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "Network.h"
#include "FragStack.h"
#include "PacketController.h"

CPacketController::CPacketController(CClient *client)
{
	m_pClient = client;
	m_pPeer = client->GetHostAddress();

	m_bEvil = FALSE;

	GenerateCRCs(0xAC2DAC2D, 0xAC2DAC2D, m_out.servercrc, m_in.clientcrc);
}

CPacketController::~CPacketController()
{
	EraseInternal(&m_in.fragstack); //!!FIXME
	EraseInternalA(&m_in.blobstack);
	EraseInternalA(&m_out.blobcache);
	EraseInternalA(&m_out.fragqueue);
}

template<class T>
void CPacketController::EraseInternal(T *data)
{
	if (data->empty())
		return;
	for (T::iterator it = data->begin(); it != data->end(); it++)
	{
		delete (*it);
	}
	data->clear();
}

template<class T>
void CPacketController::EraseInternalA(T *data)
{
	if (data->empty())
		return;
	for (T::iterator it = data->begin(); it != data->end(); it++)
	{
		delete[](*it);
	}
	data->clear();
}

DWORD CPacketController::GetNextEvent(void)
{
	return (++m_out.event_counter);
}

DWORD CPacketController::GetLastEvent(void)
{
	return (--m_out.event_counter);
}

void CPacketController::ResetEvent(void)
{
	m_out.event_counter = 0;
}

DWORD CPacketController::GetNextSequence(void)
{
	return (++m_out.sequence);
}

DWORD CPacketController::GetLastSequence(void)
{
	return (--m_out.sequence);
}

BOOL CPacketController::SendMessage(void *data, DWORD length, WORD group)
{
	DWORD dwRPC = 0x80000000;
	if (length >= 4)
	{
		DWORD dwMessageID = *((DWORD *)data);
		switch (dwMessageID)
		{
		case 0x0000F748:	dwRPC = 0x0023CED7; break;
		case 0x0000F74C:	dwRPC = 0x0023CE75;	break;
		case 0x0000F658:	dwRPC = 0x030BA655; break;
		case 0x0000F7E1:	dwRPC = 0x030BA656; break;

		case 0x0000F7E2:	dwRPC = 0x030B5C8D; break;
		}
	}


	//Every fragment has a unique sequence.
	DWORD dwFragSequence = ++m_out.fragment_counter;

	//Calculate the number of fragments necessary for the message.
	WORD wFragCount = (WORD)(length / MAX_FRAGMENT_LEN);
	if (length % MAX_FRAGMENT_LEN) wFragCount++;

	WORD wFragIndex = 0;
	WORD wFragLength = 0;
	while (wFragIndex < wFragCount)
	{
		if (wFragCount != (wFragIndex + 1))
			wFragLength = MAX_FRAGMENT_LEN;
		else
		{
			wFragLength = (WORD)(length % MAX_FRAGMENT_LEN);
			if (!wFragLength)
				wFragLength = MAX_FRAGMENT_LEN;
		}

		FragHeader_s header;
		{
			header.dwSequence = dwFragSequence;
			header.dwID = dwRPC;
			header.wCount = wFragCount;
			header.wSize = sizeof(FragHeader_s) + wFragLength;
			header.wIndex = wFragIndex;
			header.wGroup = group;
		}

		QueueFragment(&header, &((BYTE *)data)[wFragIndex * MAX_FRAGMENT_LEN]);

		wFragIndex++;
	}

	return TRUE;
}

void CPacketController::QueueFragment(FragHeader_s *header, BYTE *data)
{
	FragPacket_s *fragment = (FragPacket_s *)new BYTE[header->wSize];

	memcpy(&fragment->header, header, sizeof(FragHeader_s));
	memcpy(&fragment->data, data, header->wSize - sizeof(FragHeader_s));

	m_out.fragqueue.push_back(fragment);
}

void CPacketController::Cleanup()
{
	//Delete and remove whatever has been processed or successful sent.
	//The fragments that are complete have already been processed.
	for (std::list<FragmentStack *>::iterator it = m_in.fragstack.begin(); it != m_in.fragstack.end();)
	{
		FragmentStack *poo = *it;
		if (poo->IsComplete())
		{
			delete poo;
			it = m_in.fragstack.erase(it);
		}
		else
			it++;
	}

	for (std::list<BlobPacket_s *>::iterator it = m_in.blobstack.begin(); it != m_in.blobstack.end();)
	{
		BlobPacket_s *poo = *it;
		if (poo->header.dwSequence <= m_in.activesequence)
		{
			delete[] poo;
			it = m_in.blobstack.erase(it);
		}
		else
			it++;
	}

	//Cached blobs to flush??
	for (std::list<BlobPacket_s *>::iterator it = m_out.blobcache.begin(); it != m_out.blobcache.end();)
	{
		BlobPacket_s *poo = *it;
		if (poo->header.dwSequence <= m_in.flushsequence)
		{
			delete[] poo;
			it = m_out.blobcache.erase(it);
		}
		else
			it++;
	}
}

void CPacketController::ThinkInbound(void)
{
	//Check if we have sequences available for processing.
	DWORD dwDesired = m_in.activesequence + 1;

iterate:
	if (dwDesired <= m_in.sequence)
	{
		for (BlobList::iterator it = m_in.blobstack.begin(); it != m_in.blobstack.end(); it++)
		{
			BlobPacket_s *p = *it;
			if (dwDesired == p->header.dwSequence)
			{
				ProcessBlob(p);
				m_in.lastactivity = g_pGlobals->Time();
				delete[] p;

				m_in.blobstack.erase(it);

				m_in.activesequence = dwDesired++;
				goto iterate;
			}
		}
	}

	if ((dwDesired + 5 <= m_in.sequence) && ((m_in.lastrequest + 3) < g_pGlobals->Time()))
	{
		//request lost sequences
		std::vector<DWORD> lostSequences;
		lostSequences.push_back(dwDesired);
		DWORD lowBound = dwDesired + 1;
		DWORD highBound = m_in.sequence;
		for (DWORD check = lowBound; check < highBound; check++)
		{
			BlobList::iterator it = m_in.blobstack.begin();
			for (; it != m_in.blobstack.end(); it++)
			{
				BlobPacket_s *p = *it;
				if (check == p->header.dwSequence)
					break;
			}
			if (it == m_in.blobstack.end())
				lostSequences.push_back(check);
		}

		DWORD dwCount = (DWORD)lostSequences.size();
		CREATEBLOB(p, sizeof(DWORD) + dwCount * sizeof(DWORD));

		*((DWORD *)p->data) = lostSequences.size();
		DWORD *requestPos = ((DWORD *)p->data) + 1;

		std::vector<DWORD>::iterator requestIt = lostSequences.begin();
		for (; requestIt != lostSequences.end(); requestIt++) {
			*requestPos = *requestIt;
			requestPos++;
		}

		g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_REQUESTLOST, NULL, GetElapsedTime());

		DELETEBLOB(p);
		m_in.lastrequest = g_pGlobals->Time();
		LOG(Network, Verbose, "Requesting %d lost packets for %s..\n", lostSequences.size(), m_pClient->GetDescription());
	}
}

void CPacketController::ThinkOutbound(void)
{
	//Now for the output
	double time = g_pGlobals->Time();

	if (0)//m_out.loginsyncs < 2)
	{
		/*
		if (m_out.loginsyncs >= 0)
		{
			//if ((m_out.lastloginsync + 2.0f) < time)
			//	PerformLoginSync();
		}
		*/
	}
	else
	{
		if ((m_out.lastflush + 3.0f) < time)
			FlushPeerCache();

		/*
		if ((m_out.lasttimeupdate + 3.0f) < time)
			UpdatePeerTime();
			*/

		if (!m_out.fragqueue.empty())
			FlushFragments();
	}
}

void CPacketController::Think(void)
{
	if (!IsAlive())
		return;

	// Handle the input first.
	ThinkInbound();

	// Cleanup before we clutter the cache with new stuff.
	Cleanup();

	// Handle the output last.
	ThinkOutbound();

	if ((g_pGlobals->Time() - m_in.lastactivity) >= 30.0f)
	{
		Kill(__FILE__, __LINE__);
	}
}

BOOL CPacketController::HasConnection()
{
	return TRUE; //  (m_out.loginsyncs >= 2) ? TRUE : FALSE;
}

void CPacketController::PerformLoginSync()
{
	/* PUT THIS BACK IN?
	m_out.lastloginsync = g_pGlobals->Time();

	CREATEBLOB( LoginSync, 0xE6 );
	memset( &LoginSync->data[8], 0, 0xDE );
	*((double *)LoginSync->data) = g_pGlobals->Time();

	g_pNetwork->SendConnectlessBlob(m_pPeer, LoginSync, BT_CONNECTIONACK, NULL, GetElapsedTime());
	DELETEBLOB( LoginSync );
	*/
}

void CPacketController::FlushPeerCache()
{
	m_out.lastflush = g_pGlobals->Time();

	//
	CREATEBLOB(p, sizeof(DWORD));

	*((DWORD *)p->data) = m_in.activesequence;

	g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_ACKSEQUENCE, m_out.sequence, GetElapsedTime());
	DELETEBLOB(p);
}

void CPacketController::UpdatePeerTime()
{
	/* Piggy back instead

	m_out.lasttimeupdate = g_pGlobals->Time();
	//
	CREATEBLOB(tupdate, sizeof(double));
	*((double *)tupdate->data) = g_pGlobals->Time();

	BlobHeader_s *header = &tupdate->header;

	header->dwSequence = m_out.sequence;
	header->dwFlags = BT_TIMEUPDATE | BT_USES_CRC;
	header->dwCRC = 0;
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	DWORD dwXOR = GetSendXORVal(m_out.servercrc);
	header->dwCRC = BlobCRC(tupdate, dwXOR);

	//Off you go.
	g_pNetwork->SendPacket(m_pPeer, tupdate, BLOBLEN(tupdate));

	//Cache for later use.
	header->dwCRC = dwXOR;
	m_out.blobcache.push_back(tupdate);
	*/
}

WORD CPacketController::GetElapsedTime()
{
	return (WORD)((g_pGlobals->Time() - m_in.logintime) * 2);
}

//This is a generic handler for malicious clients.
void CPacketController::EvilClient(const char* szSource, DWORD dwLine, BOOL bKill)
{
#ifdef _DEBUG
	if (szSource)
		LOG(Temp, Normal, "Evil client @ %u of %s!!!\n", dwLine, szSource);
#endif

	if (bKill && IsAlive())
	{
		Kill(szSource, dwLine);
	}
}

void CPacketController::ProcessBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	DWORD dwSequence = header->dwSequence;
	DWORD dwFlags = header->dwFlags;
	DWORD dwCRC = header->dwCRC;
	DWORD dwSize = header->wSize;
	BYTE* pbData = blob->data;

	if (dwFlags & BT_REQUESTLOST) //0x00000002
	{
		if (dwSize >= 4)
		{
			//LOG(Temp, Normal, "Client requesting lost packets.\n");
			DWORD dwLostPackets = *((DWORD *)pbData);

			pbData += sizeof(DWORD);
			dwSize -= sizeof(DWORD);

			if (dwSize >= (dwLostPackets * sizeof(DWORD)))
			{
				DWORD dwDenySize = 0;
				DWORD dwDenyBlobs[0x50];
				for (unsigned int i = 0; i < dwLostPackets; i++)
				{
					DWORD dwRequested = ((DWORD *)pbData)[i];

					if (dwRequested < 2 || dwRequested > m_out.sequence)
					{
						// Sequence out of range.
						FlagEvilClient();
						continue;
					}

					//Find the requested sequence in the cache.
					BlobList::iterator it;
					for (it = m_out.blobcache.begin(); it != m_out.blobcache.end(); it++)
					{
						BlobPacket_s *p = *it;
						if (dwRequested == p->header.dwSequence)
						{
							ResendBlob(p);
							break;
						}
					}

					if (it == m_out.blobcache.end())
					{
						if (dwDenySize < 0x50)
						{
							dwDenyBlobs[dwDenySize] = dwRequested;
							dwDenySize++;
						}
						else
						{
							// Too many
							FlagEvilClient();
						}
					}
				}

				if (dwDenySize > 0)
				{

					DWORD dwLength = dwDenySize * sizeof(DWORD);
					CREATEBLOB(p, sizeof(DWORD) + dwLength);

					memcpy(p->data, &dwDenySize, sizeof(DWORD));
					memcpy(p->data + sizeof(DWORD), dwDenyBlobs, dwLength);

					g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_DENY, NULL, GetElapsedTime());

					DELETEBLOB(p);
				}

				pbData += sizeof(DWORD) * dwLostPackets;
				dwSize -= sizeof(DWORD) * dwLostPackets;
			}
			else
			{
				FlagEvilClient();
				return;
			}
		}
		else
		{
			FlagEvilClient();
			return;
		}
	}

	if (dwFlags & BT_DENY) //0x00000008
	{
		if (dwSize >= 4)
		{
			DWORD dwDeniedCount = *((DWORD *)pbData);

			pbData += sizeof(DWORD);
			dwSize -= sizeof(DWORD);

			if (dwSize == (dwDeniedCount * sizeof(DWORD)))
			{
				unsigned int i = 0;
				while (i < dwSize)
				{
					DWORD dwDenied = ((DWORD *)pbData)[i];

					if (dwDenied <= m_in.sequence)
					{
						if (dwDenied == (m_in.activesequence + 1))
						{
							m_in.activesequence++;
							i = 0;
						}
					}
					//else
					//	FlagEvilClient(__FILE__, __LINE__);

					i++;
				}
			}
			else
				FlagEvilClient();
		}
		else
			FlagEvilClient();

		return;
	}

	if (dwFlags & BT_ACKSEQUENCE)
	{
		if (dwSize < 4)
		{
			FlagEvilClient();
			return;
		}
		
		m_in.flushsequence = *((DWORD *)pbData);

		dwSize -= 4;
		pbData += 4;
	}

	/*
	if (dwFlags & BT_LOGIN)
	{
		if (m_out.loginsyncs < 0)
			m_out.loginsyncs = 0;

		return;
	}
	*/

	/*
	if (dwFlags & BT_CONNECTIONACK) //0x00000040
	{
		if (m_out.loginsyncs >= 0)
		{
			m_out.loginsyncs++;
			if (m_out.loginsyncs < 2)
			{
				m_in.lastactivity = g_pGlobals->Time();
				// PerformLoginSync();
			}
		}

		return;
	}
	*/

	if (dwFlags & BT_TIMEUPDATE) //0x00100000
	{
#ifdef _DEBUG
		//if (dwFlags & BT_FRAGMENTS)
		//	LOG(Temp, Normal, "Fragments? %s %u\n", __FILE__, __LINE__);
#endif
		if (dwSize < 8)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			//Time critical.
			/*
			const double gear_tolerance = 1.0f; // Cheater!! =)

			if ( (g_pGlobals->Time() + gear_tolerance) < *((double *)pbData) )
			{
				const char* account = m_pClient->GetAccount( );
				SOCKADDR_IN *ip = m_pClient->GetHostAddress( );

				LOG(Temp, Normal, "Detected client(%s @ %s) using gear. Killing!\n", (account ? account : "???"), (ip ? inet_ntoa(ip->sin_addr) : "???") );

				g_pNetwork->KickClient( m_pClient->GetIndex() );
			}

			return;
			*/
		}

		dwSize -= 8;
		pbData += 8;
	}

	if (dwFlags & BT_ECHOREQUEST)
	{
		if (dwSize < 4)
		{
			FlagEvilClient();
			return;
		}

		DWORD echoValue = *((DWORD *)pbData);

		/*
		CREATEBLOB(tupdate, sizeof(DWORD));
		*((DWORD *)tupdate->data) = echoValue;

		BlobHeader_s *header = &tupdate->header;

		header->dwSequence = m_out.sequence;
		header->dwFlags = BT_ECHORESPONSE | BT_USES_CRC;
		header->dwCRC = 0;
		header->wRecID = g_pNetwork->GetServerID();
		header->wTime = GetElapsedTime();
		header->wTable = 0x01;

		DWORD dwXOR = GetSendXORVal(m_out.servercrc);
		header->dwCRC = BlobCRC(tupdate, dwXOR);

		//Off you go.
		g_pNetwork->SendPacket(m_pPeer, tupdate, BLOBLEN(tupdate));

		//Cache for later use.
		header->dwCRC = dwXOR;
		m_out.blobcache.push_back(tupdate);
		*/

		CREATEBLOB(tupdate, sizeof(DWORD));
		*((DWORD *)tupdate->data) = echoValue;

		g_pNetwork->SendConnectlessBlob(m_pPeer, tupdate, BT_ECHORESPONSE, m_out.sequence, GetElapsedTime());

		pbData += sizeof(DWORD);
		dwSize -= sizeof(DWORD);
	}

	if (dwFlags & BT_ECHORESPONSE)
	{
		if (dwSize < 8)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			pbData += sizeof(float);
			pbData += sizeof(float);
		}

		dwSize -= 8;
	}

	if (dwFlags & BT_FLOW)
	{
		if (dwSize < 6)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			DWORD dwBytes = *((DWORD *)pbData);
			pbData += sizeof(DWORD);
			WORD wElapsed = *((WORD *)pbData);
			pbData += sizeof(WORD);

			//LOG(Temp, Normal, "Flow: %u bytes over %u seconds\n", dwBytes, wElapsed );
		}

		dwSize -= 6;
	}

	if (dwFlags & BT_FRAGMENTS)
	{
		while (dwSize >= sizeof(FragHeader_s))
		{
			FragPacket_s *frag = (FragPacket_s *)pbData;

			if (dwSize < frag->header.wSize)
				break;

			pbData += frag->header.wSize;
			dwSize -= frag->header.wSize;

			BYTE* data = frag->data;
			DWORD length = frag->header.wSize - sizeof(FragHeader_s);

			if (frag->header.wCount == 1)
			{
				ProcessMessage(data, length, frag->header.wGroup);
			}
			else
			{
				FragmentList::iterator it;
				DWORD dwSequence = frag->header.dwSequence;
				DWORD dwLogicalID = frag->header.dwID;

				if (dwLogicalID != 0x80000000)
					LOG(Temp, Normal, "Client sent ID %08X\n");

				bool bCompleted = false;
				for (it = m_in.fragstack.begin(); it != m_in.fragstack.end(); it++)
				{
					FragmentStack *s = *it;
					if (s->m_dwSequence == dwSequence && s->m_dwID == dwLogicalID)
					{
						s->AddFragment(frag);

						if (s->IsComplete())
						{
							ProcessMessage(s->GetData(), s->GetLength(), frag->header.wGroup);
							delete s;

							m_in.fragstack.erase(it);
							bCompleted = true;
						}
						break;
					}
				}

				
				if (!bCompleted)
				{					
					if (it == m_in.fragstack.end())
					{
						// No existing group matches, create one
						FragmentStack *s = new FragmentStack(frag);
						m_in.fragstack.push_back(s);
					}
				}
			}
		}
	}
}

void CPacketController::ProcessMessage(BYTE *data, DWORD length, WORD group)
{
	if (m_pClient->IsAlive())
		m_pClient->ProcessMessage(data, length, group);
}

void CPacketController::IncomingBlob(BlobPacket_s *blob)
{
	/*
	Client's process goes something like this:
	(args: blob, size)

	DWORD dwHeaderCRC = CalcTransportCRC( buffer );
	blob.crc -= dwHeaderCRC;

	DWORD dwFlags = blob.flags;
	if ( dwFlags & 0x00000080 ) //account packet
	{
		//??
		UnlockThread();
		return 1;
	}
	else if ( dwFlags & 0x00000008 ) //deny sequences
	{
		blob.crc -= GetMagicNumber( blob.data, blob.size, FALSE );

		if ( !blob.crc )
		{
			UnlockThread();
			return 3;
		}
		else
			return 2;
	}
	else if ( dwFlags & 0x00100000 ) //update time
	{
		if ( size >= 8 )
		{
		}
	}
	else
	{
		if ( dwFlags & 0x00800000 ) // error
		{
			if ( size >= 4 )
			{
			}
		}
		else if ( )//etc.
		{
		}
	}
	*/

	//No CRC checks as of yet.
	BlobHeader_s *header = &blob->header;

	DWORD dwSequence = header->dwSequence;
	DWORD dwFlags = header->dwFlags;
	DWORD dwCRC = header->dwCRC;
	DWORD dwSize = header->wSize;
	BYTE* pbData = blob->data;

	if (dwSequence > m_in.sequence)	m_in.sequence = dwSequence;

	if (dwFlags & BT_DISCONNECT)
	{
		Kill(__FILE__, __LINE__);
		return;
	}
	
	if (dwFlags & BT_REQUESTLOST)
	{
		ProcessBlob(blob);
		return;
	}

	/*
	if (dwFlags & BT_ACKSEQUENCE)
	{
		if (dwSize < 4)
			FlagEvilClient();
		else
			m_in.flushsequence = *((DWORD *)blob->data);

		if (dwSequence == m_in.activesequence)
		{
			m_in.lastactivity = g_pGlobals->Time();
		}

		return;
	}
	*/

	if (dwSequence == 0x00000001)
	{
		//if (dwFlags == BT_NULL)
		//	return;
	}

	if (dwSequence == 0x00000002)
	{
		//if (dwFlags & BT_CRCUPDATE)
		//	return;
	}

	if (!dwSequence)
	{
		ProcessBlob(blob);
		return;
	}
	if (dwSequence <= m_in.activesequence) return;

	//Cache this blob, for later use.
	DUPEBLOB(p, blob);
	m_in.blobstack.push_back(p);
}

void CPacketController::ResendBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	header->dwFlags |= BT_RESENT;

	DWORD dwXOR = header->dwCRC;
	header->dwCRC = BlobCRC(blob, dwXOR);

	//Off you go.
	g_pNetwork->SendPacket(m_pPeer, blob, BLOBLEN(blob));

	//Keep in cache.
	header->dwCRC = dwXOR;
}

void CPacketController::SendFragmentBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	header->dwSequence = GetNextSequence();
	header->dwFlags = BT_FRAGMENTS | BT_USES_CRC;
	header->dwCRC = 0;
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	DWORD dwXOR = GetSendXORVal(m_out.servercrc);
	header->dwCRC = BlobCRC(blob, dwXOR);

	//Off you go.
	g_pNetwork->SendPacket(m_pPeer, blob, BLOBLEN(blob));

	//Cache for later use.
	header->dwCRC = dwXOR;
	m_out.blobcache.push_back(blob);
}


//Sends all the queued fragments.
void CPacketController::FlushFragments(void)
{
	BlobPacket_s *blob = (BlobPacket_s *)new BYTE[MAX_BLOB_LEN];
	blob->header.wSize = 0;

	for (std::list<FragPacket_s *>::iterator it = m_out.fragqueue.begin(); it != m_out.fragqueue.end(); it++)
	{
		FragPacket_s *edible = *it;
		if ((blob->header.wSize + edible->header.wSize) < (MAX_BLOB_LEN - sizeof(BlobHeader_s)))
		{
			BYTE *dataPos = &blob->data[blob->header.wSize];
			WORD wSize = edible->header.wSize;
			memcpy(dataPos, edible, wSize);
			blob->header.wSize += wSize;
		}
		else
		{
			//This one is full, let's send it and make a new one.
			SendFragmentBlob(blob);

			blob = (BlobPacket_s *)new BYTE[MAX_BLOB_LEN];

			WORD wSize = blob->header.wSize = edible->header.wSize;
			memcpy(blob->data, edible, wSize);
		}

		delete[] edible;
	}
	SendFragmentBlob(blob);

	m_out.fragqueue.clear();
}




