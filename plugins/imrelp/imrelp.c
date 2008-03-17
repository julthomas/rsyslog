/* imrelp.c
 *
 * This is the implementation of the RELP input module.
 *
 * File begun on 2008-03-13 by RGerhards
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */

#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <relp.h>
#include "rsyslog.h"
#include "syslogd.h"
#include "cfsysline.h"
#include "module-template.h"
#include "net.h"

MODULE_TYPE_INPUT

/* static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(net)

/* Module static data */
static relpEngine_t *pRelpEngine;	/* our relp engine */


/* config settings */
static int iTCPSessMax = 200; /* max number of sessions */


#if 0
/* callbacks */
/* this shall go into a specific ACL module! */
static int
isPermittedHost(struct sockaddr *addr, char *fromHostFQDN, void __attribute__((unused)) *pUsrSrv,
	        void __attribute__((unused)) *pUsrSess)
{
	return net.isAllowedSender(net.pAllowedSenders_TCP, addr, fromHostFQDN);
}


static int*
doOpenLstnSocks(tcpsrv_t *pSrv)
{
	ISOBJ_TYPE_assert(pSrv, tcpsrv);
	return tcpsrv.create_tcp_socket(pSrv);
}


static int
doRcvData(tcps_sess_t *pSess, char *buf, size_t lenBuf)
{
	int state;
	assert(pSess != NULL);

	state = recv(pSess->sock, buf, lenBuf, 0);
	return state;
}

static rsRetVal
onRegularClose(tcps_sess_t *pSess)
{
	DEFiRet;
	assert(pSess != NULL);

	/* process any incomplete frames left over */
	tcps_sess.PrepareClose(pSess);
	/* Session closed */
	tcps_sess.Close(pSess);
	RETiRet;
}


static rsRetVal
onErrClose(tcps_sess_t *pSess)
{
	DEFiRet;
	assert(pSess != NULL);

	tcps_sess.Close(pSess);
	RETiRet;
}

/* ------------------------------ end callbacks ------------------------------ */
#endif // #if 0


static rsRetVal addListener(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	DEFiRet;
	if(pRelpEngine == NULL) {
		CHKiRet(relpEngineConstruct(&pRelpEngine));
		CHKiRet(relpEngineSetDbgprint(pRelpEngine, dbgprintf));
	}

	CHKiRet(relpEngineAddListner(pRelpEngine, pNewVal));

finalize_it:
	RETiRet;
}

/* This function is called to gather input.
 */
BEGINrunInput
CODESTARTrunInput
	/* TODO: we must be careful to start the listener here. Currently, tcpsrv.c seems to
	 * do that in ConstructFinalize
	 */
	iRet = relpEngineRun(pRelpEngine);
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
	/* first apply some config settings */
	//net.PrintAllowedSenders(2); /* TCP */
	if(pRelpEngine == NULL)
		ABORT_FINALIZE(RS_RET_NO_RUN);
finalize_it:
ENDwillRun


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
#if 0
	if(net.pAllowedSenders_TCP != NULL) {
		net.clearAllowedSenders(net.pAllowedSenders_TCP);
		net.pAllowedSenders_TCP = NULL;
	}
#endif
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	if(pRelpEngine != NULL)
		iRet = relpEngineDestruct(&pRelpEngine);

	/* release objects we used */
	objRelease(net, LM_NET_FILENAME);
ENDmodExit


static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	iTCPSessMax = 200;
	return RS_RET_OK;
}



BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	pRelpEngine = NULL;
	/* request objects we use */
	CHKiRet(objUse(net, LM_NET_FILENAME));

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputrelpserverrun", 0, eCmdHdlrGetWord,
				   addListener, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputrelpmaxsessions", 0, eCmdHdlrInt,
				   NULL, &iTCPSessMax, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit


/* vim:set ai:
 */
