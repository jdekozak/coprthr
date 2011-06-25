/* xcl_context.c 
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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


#include <CL/cl.h>

#include "xcl_structs.h"
#include "context.h"


#define MAXPROPLEN 10

static int __find_context_property(cl_context_properties*, 
	cl_context_properties, void*);


// Context APIs  


cl_context 
clCreateContext(
//	cl_context_properties* prop,
	const cl_context_properties* prop,
	cl_uint ndev, 
	const cl_device_id* devices, 
	void (*pfn_notify) (const char*, const void*, size_t, void*),
	void* user_data,
	cl_int* err_ret
)
{
	DEBUG(__FILE__,__LINE__,"clCreateContext");

	size_t sz;

	if (!devices || ndev==0) __error_return(CL_INVALID_VALUE,cl_context);

	if (!pfn_notify && user_data) __error_return(CL_INVALID_VALUE,cl_context);

	/* XXX several checks are not implemented here -DAR */


	cl_platform_id platformid;

	if (!__find_context_property(prop,CL_CONTEXT_PLATFORM,&platformid)) {

		WARN(__FILE__,__LINE__,"clCreateContext: no platformid, using default");

		__do_get_default_platformid(&platformid);
	}


	struct _cl_context* ctx 
		= (struct _cl_context*)malloc(sizeof(struct _cl_context));

	
	if (ctx) {

		__init_context(ctx);

		int n = 0;
		while(n<MAXPROPLEN && prop[n]) n+=2;
		__clone(ctx->prop,prop,n+1,cl_context_properties);

		ctx->ndev = ndev;
		__clone(ctx->devices,devices,ndev,cl_device_id);
	
		ctx->pfn_notify = pfn_notify;
		ctx->user_data = user_data;

		__do_create_context(ctx);

		ctx->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_context);;

	__success();

	return(ctx);
}


	 
cl_context 
clCreateContextFromType(
	const cl_context_properties* prop, 
  	cl_device_type devtype, 
	void (*pfn_notify) (const char*, const void*, size_t, void*),
	void* user_data, 
	cl_int* err_ret	
)
{
	DEBUG(__FILE__,__LINE__,"clCreateContextFromType");

	if (!pfn_notify && user_data) __error_return(CL_INVALID_VALUE,cl_context);

	/* XXX several checks are not implemented here -DAR */

	int i;

	cl_platform_id platformid;

	if (!__find_context_property(prop,CL_CONTEXT_PLATFORM,&platformid)) {

		WARN(__FILE__,__LINE__,"clCreateContext: no platformid, using default");

		__do_get_default_platformid(&platformid);

	}


	cl_uint n;

//   __do_get_ndevices(platformid,&n);
   __do_get_ndevices(platformid,devtype,&n);

	DEBUG(__FILE__,__LINE__,"clCreateContextFromType: platform ndev=%d",n);

   if (n == 0) __error_return(CL_DEVICE_NOT_FOUND,cl_context);

	cl_device_id* devices = (cl_device_id*)calloc(n,sizeof(cl_device_id));

	__do_get_devices(platformid,devtype,n,devices);

	cl_uint ndev = 0;

	for(i=0;i<n;i++) {

		DEBUG(__FILE__,__LINE__,
			"clCreateContextFromType: compare %x %x",
			__resolve_devid(devices[i],devtype),devtype);

		if (__resolve_devid(devices[i],devtype)&devtype) 
			memcpy(devices+(ndev++),devices+i,sizeof(cl_device_id));

	}

	DEBUG(__FILE__,__LINE__,"clCreateContextFromType: matching ndev=%d",ndev);

	if (ndev == 0) __error_return(CL_DEVICE_NOT_FOUND,cl_context);

	if (!(devices = realloc(devices,ndev*sizeof(cl_device_id)))) 
		__error_return(CL_OUT_OF_HOST_MEMORY,cl_context);
		

	struct _cl_context* ctx 
		= (struct _cl_context*)malloc(sizeof(struct _cl_context));


	if (ctx) {

		__init_context(ctx);

		int n; for( n=0; n<MAXPROPLEN && prop[n]; n+=2);
		__clone(ctx->prop,prop,n+1,cl_context_properties);

		ctx->ndev = ndev;
		ctx->devices = devices;
	
		ctx->pfn_notify = pfn_notify;
		ctx->user_data = user_data;

		__do_create_context(ctx);

		ctx->refc = 1;

	} else {

		__free(devices);

		__error_return(CL_OUT_OF_HOST_MEMORY,cl_context);

	}

	__success();

	return(ctx);
}


cl_int 
clRetainContext(cl_context	ctx )
{
	DEBUG(__FILE__,__LINE__,"clRetainContext");

	if (__invalid_context(ctx)) return(CL_INVALID_CONTEXT);

	++ctx->refc;

	return(CL_SUCCESS);
}


cl_int 
clReleaseContext(cl_context ctx )
{
	DEBUG(__FILE__,__LINE__,"clReleaseContext");

	if (__invalid_context(ctx)) return(CL_INVALID_CONTEXT);

	if (--ctx->refc == 0) {

		__do_release_context(ctx);

		__free_context(ctx);

	}

	return(CL_SUCCESS);
}

	 
cl_int 
clGetContextInfo(
	cl_context ctx, cl_context_info param_name, 
	size_t param_sz, void* param_val, size_t* param_sz_ret
)
{

	if (__invalid_context(ctx)) return(CL_INVALID_CONTEXT);

	size_t sz;

	switch (param_name) {

		case CL_CONTEXT_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&ctx->refc);

			break;

		case CL_CONTEXT_DEVICES:

			__case_get_param((ctx->ndev)*sizeof(cl_device_id),ctx->devices);

			break;

		case CL_CONTEXT_PROPERTIES:
			WARN(__FILE__,__LINE__,"clGetContextInfo: unsupported");
			/* XXX context properties are not supported here yet. -DAR */

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}



static int __find_context_property( 
	cl_context_properties* props, cl_context_properties prop, void* val 
)
{
	int n = 0;

	while( n < 2*MAXPROPLEN) {

		cl_context_properties p = props[n++];

		DEBUG(__FILE__,__LINE__,"__find_context_property: (%x) p=%x",prop,p);

		if (p == 0) return(0);

		cl_context_properties v = props[n++];

		DEBUG(__FILE__,__LINE__,"__find_context_property: v=%x",v);

		if (p == prop) { *(cl_context_properties*)val = v; 
DEBUG(__FILE__,__LINE__,"__find_context_property: match"); return(1); 
}

	}

	WARN(__FILE__,__LINE__, "__find_conext_property:"
		" cl_context_properties list not terminated");

	return(0);
}


