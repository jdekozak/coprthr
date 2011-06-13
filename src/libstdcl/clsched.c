/* clsched.c
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */


/* XXX to do, add err code checks, other safety checks * -DAR */
/* XXX to do, clvplat_destroy should automatically release all txts -DAR */

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
//#include <dlfcn.h>

#include <CL/cl.h>
//#include "stdcl.h"
#include "util.h"
#include "clsched.h"



#define __error(e) do { errno=e; return(-1); } while(0); 


LIBSTDCL_API
cl_event clfork(
	CONTEXT* cp, cl_uint devnum, 
	cl_kernel krn, struct clndrange_struct* ndr, int flags
)
{
	
	if (cp->kev[devnum].nev==STDCL_EVENTLIST_MAX) return((cl_event)0);

	/* XXX here you should check flags to ensure sufficient and valid -DAR */

	cl_event* evp = (cp->kev[devnum].ev_first==cp->kev[devnum].ev_free)? 
		0 : cp->kev[devnum].ev + cp->kev[devnum].ev_free -1;

	cl_event ev;

	DEBUG(__FILE__,__LINE__,"clfork: devnum=%d\n",devnum);
	DEBUG(__FILE__,__LINE__,"clfork: kev,evp %p,%p\n",cp->kev[devnum].ev,evp);

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	 DEBUG(__FILE__,__LINE__,"clfork: ndr.dim=%d\n",ndr->dim);

#if defined(CL_VERSION_1_1)
	 DEBUG(__FILE__,__LINE__,"clfork: using CL_VERSION_1_1");
#endif

#if defined(CL_VERSION_1_1)
	 DEBUG(__FILE__,__LINE__,"clfork: ndr.gtid_offset=%d %d %d %d\n",
		ndr->gtid_offset[0],ndr->gtid_offset[1],
		ndr->gtid_offset[2],ndr->gtid_offset[3]);
#endif
	 DEBUG(__FILE__,__LINE__,"clfork: ndr.gtid=%d %d %d %d\n",
		ndr->gtid[0],ndr->gtid[1],ndr->gtid[2],ndr->gtid[3]);
	 DEBUG(__FILE__,__LINE__,"clfork: ndr.ltid=%d %d %d %d\n",
		ndr->ltid[0],ndr->ltid[1],ndr->ltid[2],ndr->ltid[3]);

	if (flags & CL_FAST) {

#if defined(CL_VERSION_1_1)
	 DEBUG(__FILE__,__LINE__,"clfork: using CL_VERSION_1_1");
#endif

	int err = clEnqueueNDRangeKernel(
#if defined(CL_VERSION_1_1)
		cp->cmdq[devnum],krn,ndr->dim,ndr->gtid_offset,ndr->gtid,ndr->ltid,
#else
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
#endif
		0,0,0
	);
	switch(err)
	  {
	  case CL_INVALID_PROGRAM_EXECUTABLE:
	    ERROR(__FILE__,__LINE__,
		    "there is no successfully built program executable available for device associated with command_queue.");
	    break;
	  case CL_INVALID_COMMAND_QUEUE:
	    ERROR(__FILE__,__LINE__,
		    "command_queue is not a valid command-queue.");
	    break;
	  case CL_INVALID_KERNEL:
	    ERROR(__FILE__,__LINE__,
		    "kernel is not a valid kernel object.");
	    break;
	  case CL_INVALID_CONTEXT:
	    ERROR(__FILE__,__LINE__,
		    "context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same.");
	    break;
	  case CL_INVALID_KERNEL_ARGS:
	    ERROR(__FILE__,__LINE__,
		    "the kernel argument values have not been specified.");
	    break;
	  case CL_INVALID_WORK_DIMENSION:
	    ERROR(__FILE__,__LINE__,
		    "work_dim is not a valid value (i.e. a value between 1 and 3).");
	    break;
	  case CL_INVALID_WORK_GROUP_SIZE:
	    ERROR(__FILE__,__LINE__,
		    "IF local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program source.");
	    ERROR(__FILE__,__LINE__,
		    "OR local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *... local_work_size[work_dim - 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.");
	    ERROR(__FILE__,__LINE__,
		    "OR local_work_size is NULL and the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.");
	    break;
	  case CL_INVALID_WORK_ITEM_SIZE:
	    ERROR(__FILE__,__LINE__,
		    "the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1].");
	    break;
	  case CL_INVALID_GLOBAL_OFFSET:
	    ERROR(__FILE__,__LINE__,
		    "global_work_offset is not NULL.");
	    break;
	  case CL_OUT_OF_RESOURCES:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device.");
	    break;
	  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.");
	    break;
	  case CL_INVALID_EVENT_WAIT_LIST:
	    ERROR(__FILE__,__LINE__,
		    "event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.");
	    break;
	  case CL_OUT_OF_HOST_MEMORY:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to allocate resources required by the OpenCL implementation on the host.");
	    break;
	  default:
	    break;
	  }
	if(err != CL_SUCCESS) exit(-1);
	return((cl_event)0);


	} else {

	int err = clEnqueueNDRangeKernel(
#if defined(CL_VERSION_1_1)
		cp->cmdq[devnum],krn,ndr->dim,ndr->gtid_offset,ndr->gtid,ndr->ltid,
#else
		cp->cmdq[devnum],krn,ndr->dim,0,ndr->gtid,ndr->ltid,
#endif
		(evp)?1:0,evp,&ev
	);
	switch(err)
	  {
	  case CL_INVALID_PROGRAM_EXECUTABLE:
	    ERROR(__FILE__,__LINE__,
		    "there is no successfully built program executable available for device associated with command_queue.");
	    break;
	  case CL_INVALID_COMMAND_QUEUE:
	    ERROR(__FILE__,__LINE__,
		    "command_queue is not a valid command-queue.");
	    break;
	  case CL_INVALID_KERNEL:
	    ERROR(__FILE__,__LINE__,
		    "kernel is not a valid kernel object.");
	    break;
	  case CL_INVALID_CONTEXT:
	    ERROR(__FILE__,__LINE__,
		    "context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same.");
	    break;
	  case CL_INVALID_KERNEL_ARGS:
	    ERROR(__FILE__,__LINE__,
		    "the kernel argument values have not been specified.");
	    break;
	  case CL_INVALID_WORK_DIMENSION:
	    ERROR(__FILE__,__LINE__,
		    "work_dim is not a valid value (i.e. a value between 1 and 3).");
	    break;
	  case CL_INVALID_WORK_GROUP_SIZE:
	    ERROR(__FILE__,__LINE__,
		    "IF local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program source.");
	    ERROR(__FILE__,__LINE__,
		    "OR local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *... local_work_size[work_dim - 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.");
	    ERROR(__FILE__,__LINE__,
		    "OR local_work_size is NULL and the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.");
	    break;
	  case CL_INVALID_WORK_ITEM_SIZE:
	    ERROR(__FILE__,__LINE__,
		    "the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1].");
	    break;
	  case CL_INVALID_GLOBAL_OFFSET:
	    ERROR(__FILE__,__LINE__,
		    "global_work_offset is not NULL.");
	    break;
	  case CL_OUT_OF_RESOURCES:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device.");
	    break;
	  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.");
	    break;
	  case CL_INVALID_EVENT_WAIT_LIST:
	    ERROR(__FILE__,__LINE__,
		    "event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.");
	    break;
	  case CL_OUT_OF_HOST_MEMORY:
	    ERROR(__FILE__,__LINE__,
		    "there is a failure to allocate resources required by the OpenCL implementation on the host.");
	    break;
	  default:
	    break;
	  }
	if(err != CL_SUCCESS) exit(-1);

	DEBUG(__FILE__,__LINE__,"clfork: clEnqueueNDRangeKernel err %d\n",err);

	if (flags & CL_EVENT_NOWAIT) {

		cp->kev[devnum].ev[cp->kev[devnum].ev_free++] = ev;
		cp->kev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->kev[devnum].nev;

//	} else { /* CL_EVENT_WAIT */
	} else if (flags & CL_EVENT_WAIT) { /* CL_EVENT_WAIT */

		DEBUG(__FILE__,__LINE__,"clfork: clWaitForEvents %d,%p",1,&ev);

		err = clWaitForEvents( 1, &ev);
		DEBUG(__FILE__,__LINE__,"clfork: clWaitForEvents %d",err);

//#ifdef USE_DEPRECATED_FLAGS
//		if (flags & CL_EVENT_RELEASE && !(flags & CL_EVENT_NORELEASE) ) {
//			clReleaseEvent(ev);
//			ev = (cl_event)0;
//		}
//#else
		if ( !(flags & CL_EVENT_NORELEASE) ) {
			clReleaseEvent(ev);
			ev = (cl_event)0;
		}
//#endif

	}

	}


	DEBUG(__FILE__,__LINE__,
		"clfork first,free %d,%d\n",cp->kev[devnum].ev_first,cp->kev[devnum].ev_free);
	
	return(ev);
}


LIBSTDCL_API
cl_event clwait(CONTEXT* cp, unsigned int devnum, int flags)
{
	int err;
	int n;
	cl_event* evp;

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	if (flags&CL_FAST) {
		clFinish(cp->cmdq[0]);
		return((cl_event)0);
	}

	/* XXX clwait should be responsive to these flags, fix -DAR */

//#ifdef USE_DEPRECATED_FLAGS
//	if ( !flags&CL_EVENT_RELEASE ) {
//		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
//	}
//#else
	if ( flags&CL_EVENT_NORELEASE ) {
		WARN(__FILE__,__LINE__,"clwait: ignoring CL_EVENT_NORELEASE");
	}
//#endif

	/* provide warning if no event list has been chosen */

	if (!flags&(CL_KERNEL_EVENT|CL_MEM_EVENT)) {
		WARN(__FILE__,__LINE__,"clwait: no event list specified");
	}


	DEBUG(__FILE__,__LINE__,"clwait: devnum=%d kev.ndev=%d mev.nev=%d",
		devnum,cp->kev[devnum].nev,cp->mev[devnum].nev);

	/*
	 * wait on kernel events
	 */

	if (flags&CL_KERNEL_EVENT && cp->kev[devnum].nev > 0) {

		DEBUG(__FILE__,__LINE__,
			"clwait first,free=%d,%d knev=%d\n",
			cp->kev[devnum].ev_first,cp->kev[devnum].ev_free,cp->kev[devnum].nev);

//		if (!cp->kev[devnum].nev) return((cl_event)0);

		if (cp->kev[devnum].ev_first < cp->kev[devnum].ev_free) {

			DEBUG(__FILE__,__LINE__, "clwait one event list");

			DEBUG(__FILE__,__LINE__, "clwait: clWaitForEvents(%d,%p) %p",
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first,
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first],cp->kev[devnum].ev);

DEBUG(__FILE__,__LINE__, "clwait: here");

			err = clWaitForEvents(
				cp->kev[devnum].ev_free - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first]
			);

DEBUG(__FILE__,__LINE__, "clwait: here");

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first 
				= cp->kev[devnum].ev_free = 0;

		} else if (cp->kev[devnum].ev_free > cp->kev[devnum].ev_first) {

			DEBUG(__FILE__,__LINE__, "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->kev[devnum].ev_first, 
				&cp->kev[devnum].ev[cp->kev[devnum].ev_first]
			);

			err = clWaitForEvents(cp->kev[devnum].ev_free, cp->kev[devnum].ev);

			for(evp = cp->kev[devnum].ev + cp->kev[devnum].ev_first; 
				evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) clReleaseEvent(*evp);

			for(evp = cp->kev[devnum].ev; 
				evp < cp->kev[devnum].ev + cp->kev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		} else {

			DEBUG(__FILE__,__LINE__, "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->kev[devnum].ev);

			for(evp = cp->kev[devnum].ev; evp < cp->kev[devnum].ev + STDCL_EVENTLIST_MAX; evp++)
				clReleaseEvent(*evp);

			cp->kev[devnum].nev = cp->kev[devnum].ev_first = cp->kev[devnum].ev_free = 0;

		}

	}

	if (flags&CL_MEM_EVENT && cp->mev[devnum].nev > 0) {

		DEBUG(__FILE__,__LINE__,
			"clwait first,free=%d,%d mnev=%d\n",
			cp->mev[devnum].ev_first,cp->mev[devnum].ev_free,cp->mev[devnum].nev);

//		if (!cp->mev[devnum].nev) return((cl_event)0);

		if (cp->mev[devnum].ev_first < cp->mev[devnum].ev_free) {

			DEBUG(__FILE__,__LINE__, "clwait one event list");

			DEBUG(__FILE__,__LINE__, "clwait: clWaitForEvents(%d,%p) %p",
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first,
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first],cp->mev[devnum].ev);

			err = clWaitForEvents(
				cp->mev[devnum].ev_free - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]
			);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else if (cp->mev[devnum].ev_free > cp->mev[devnum].ev_first) {

			DEBUG(__FILE__,__LINE__, "clwait two event lists");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX - cp->mev[devnum].ev_first, 
				&cp->mev[devnum].ev[cp->mev[devnum].ev_first]);

			err = clWaitForEvents(cp->mev[devnum].ev_free, cp->mev[devnum].ev);

			for(evp = cp->mev[devnum].ev + cp->mev[devnum].ev_first; 
				evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++
			) clReleaseEvent(*evp);

			for(evp = cp->mev[devnum].ev; 
				evp < cp->mev[devnum].ev + cp->mev[devnum].ev_free; evp++
			) clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		} else {

			DEBUG(__FILE__,__LINE__, "clwait full even list");

			err = clWaitForEvents(STDCL_EVENTLIST_MAX,cp->mev[devnum].ev);

			for(evp = cp->mev[devnum].ev; evp < cp->mev[devnum].ev + STDCL_EVENTLIST_MAX; evp++)
				clReleaseEvent(*evp);

			cp->mev[devnum].nev = cp->mev[devnum].ev_first 
				= cp->mev[devnum].ev_free = 0;

		}

	}
	
	return((cl_event)0);

}


LIBSTDCL_API
cl_event clwaitev(
  CONTEXT* cp, unsigned int devnum, const cl_event ev, int flags
)
{
	int err;

	/* XXX clwait should be responsive to these flags, fix -DAR */

//#ifdef USE_DEPRECATED_FLAGS
//	if ( !flags&CL_EVENT_RELEASE ) {
//		WARN(__FILE__,__LINE__,"clwait: forcing CL_EVENT_RELEASE");
//	}
//#else
	if ( flags&CL_EVENT_NORELEASE ) {
		WARN(__FILE__,__LINE__,"clwait: ignoring CL_EVENT_NORELEASE");
	}
//#endif


	err = clWaitForEvents(1,&ev);
	clReleaseEvent(ev);

	/* XXX here should force ev to 0 however, require evp -DAR */

	return(ev);
}


LIBSTDCL_API
int clflush(CONTEXT* cp, unsigned int devnum, int flags)
{

//	if (flags&CL_FAST) {
//		clFinish(cp->cmdq[0]);
//		return((cl_event)0);
//	}

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	clFlush(cp->cmdq[devnum]);

	return(0);

}

