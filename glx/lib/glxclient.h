#ifndef _GLX_client_h_
#define _GLX_client_h_

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
** $Date$ $Revision$
** $Header: //depot/main/glx/lib/glxclient.h#14 $
*/

#define NEED_REPLIES
#define NEED_EVENTS
#include <GL/glx.h>
#include "glxint.h"
#include "glxproto.h"
#include <X11/Xproto.h>
#include <X11/Xlibint.h>
#include <string.h>
#include <stdlib.h>

#define GLX_MAJOR_VERSION	1	/* current version numbers */
#define GLX_MINOR_VERSION	2

#define __GL_BOOLEAN_ARRAY	(GL_BYTE - 1)

#define __GLX_MAX_TEXTURE_UNITS 32

typedef struct __GLXcontextRec __GLXcontext;
typedef struct __GLXdisplayPrivateRec __GLXdisplayPrivate;

/************************************************************************/

#define __GL_CLIENT_ATTRIB_STACK_DEPTH 16

typedef struct __GLXpixelStoreModeRec {
    GLboolean swapEndian;
    GLboolean lsbFirst;
    GLuint rowLength;
    GLuint imageHeight;
    GLuint imageDepth;
    GLuint skipRows;
    GLuint skipPixels;
    GLuint skipImages;
    GLuint alignment;
} __GLXpixelStoreMode;

typedef struct __GLXvertexArrayPointerStateRec {
    GLboolean enable;
    void (*proc)(const void *);
    const GLubyte *ptr;
    GLsizei skip;
    GLint size;
    GLenum type;
    GLsizei stride;
} __GLXvertexArrayPointerState;

typedef struct __GLXvertArrayStateRec {
    __GLXvertexArrayPointerState vertex;
    __GLXvertexArrayPointerState normal;
    __GLXvertexArrayPointerState color;
    __GLXvertexArrayPointerState index;
    __GLXvertexArrayPointerState texCoord[__GLX_MAX_TEXTURE_UNITS];
    __GLXvertexArrayPointerState edgeFlag;
    GLint maxElementsVertices;
    GLint maxElementsIndices;
    GLint activeTexture;
} __GLXvertArrayState;

typedef struct __GLXattributeRec {
	GLuint mask;

	/*
	** Pixel storage state.  Most of the pixel store mode state is kept
	** here and used by the client code to manage the packing and
	** unpacking of data sent to/received from the server.
	*/
	__GLXpixelStoreMode storePack, storeUnpack;

	/*
	** Vertex Array storage state.  The vertex array component
	** state is stored here and is used to manage the packing of
	** DrawArrays data sent to the server.
	*/
	__GLXvertArrayState vertArray;
} __GLXattribute;

typedef struct __GLXattributeMachineRec {
	__GLXattribute *stack[__GL_CLIENT_ATTRIB_STACK_DEPTH];
	__GLXattribute **stackPointer;
} __GLXattributeMachine;

/*
** GLX state that needs to be kept on the client.  One of these records
** exist for each context that has been made current by this client.
*/
struct __GLXcontextRec {
    /*
    ** Drawing command buffer.  Drawing commands are packed into this
    ** buffer before being sent as a single GLX protocol request.  The
    ** buffer is sent when it overflows or is flushed by
    ** __glXFlushRenderBuffer.  "pc" is the next location in the buffer
    ** to be filled.  "limit" is described above in the buffer slop
    ** discussion.
    **
    ** Commands that require large amounts of data to be transfered will
    ** also use this buffer to hold a header that describes the large
    ** command.
    **
    ** These must be the first 6 fields since they are static initialized
    ** in the dummy context in glxext.c
    */
    GLubyte *buf;
    GLubyte *pc;
    GLubyte *limit;
    GLubyte *bufEnd;
    GLint bufSize;

    /*
    ** The XID of this rendering context.  When the context is created a
    ** new XID is allocated.  This is set to None when the context is
    ** destroyed but is still current to some thread. In this case the
    ** context will be freed on next MakeCurrent.
    */
    XID xid;

    /*
    ** The XID of the shareList context.
    */
    XID share_xid;

    /*
    ** Visual id.
    */
    VisualID vid;

    /*
    ** screen number.
    */
    GLint screen;

    /*
    ** GL_TRUE if the context was created with ImportContext, which
    ** means the server-side context was created by another X client.
    */
    GLboolean imported;

    /*
    ** The context tag returned by MakeCurrent when this context is made
    ** current. This tag is used to identify the context that a thread has
    ** current so that proper server context management can be done.  It is
    ** used for all context specific commands (i.e., Render, RenderLarge,
    ** WaitX, WaitGL, UseXFont, and MakeCurrent (for the old context)).
    */
    GLXContextTag currentContextTag;

    /*
    ** The rendering mode is kept on the client as well as the server.
    ** When glRenderMode() is called, the buffer associated with the
    ** previous rendering mode (feedback or select) is filled.
    */
    GLenum renderMode;
    GLfloat *feedbackBuf;
    GLuint *selectBuf;

    /*
    ** This is GL_TRUE if the pixel unpack modes are such that an image
    ** can be unpacked from the clients memory by just copying.  It may
    ** still be true that the server will have to do some work.  This
    ** just promises that a straight copy will fetch the correct bytes.
    */
    GLboolean fastImageUnpack;

    /*
    ** Fill newImage with the unpacked form of oldImage getting it
    ** ready for transport to the server.
    */
    void (*fillImage)(__GLXcontext*, GLint, GLint, GLint, GLint, GLenum,
		      GLenum, const GLvoid*, GLubyte*, GLubyte*);

    /*
    ** Client side attribs.
    */
    __GLXattribute state;
    __GLXattributeMachine attributes;

    /*
    ** Client side error code.  This is set when client side gl API
    ** routines need to set an error because of a bad enumerant or
    ** running out of memory, etc.
    */
    GLenum error;

    /*
    ** Whether this context does direct rendering.
    */
    Bool isDirect;

    /*
    ** dpy of current display for this context. Will be NULL if not
    ** current to any display, or if this is the "dummy context".
    */
    Display *currentDpy;

    /*
    ** The current drawable for this context.  Will be None if this
    ** context is not current to any drawable.
    */
    GLXDrawable currentDrawable;

    /*
    ** Constant strings that describe the server implementation
    ** These pertain to GL attributes, not to be confused with
    ** GLX versioning attributes.
    */
    GLubyte *vendor;
    GLubyte *renderer;
    GLubyte *version;
    GLubyte *extensions;

    /* Record the dpy this context was created on for later freeing */
    Display *createDpy;

    /*
    ** Maximum small render command size.  This is the smaller of 64k and
    ** the size of the above buffer.
    */
    GLint maxSmallRenderCommandSize;

    /*
    ** Major opcode for the extension.  Copied here so a lookup isn't
    ** needed.
    */
    GLint majorOpcode;
};

#define __glXSetError(gc,code) \
    if (!(gc)->error) {	       \
	(gc)->error = code;    \
    }

extern void __glFreeAttributeState(__GLXcontext *);

/************************************************************************/

/*
** The size of the largest drawing command known to the implementation
** that will use the GLXRender glx command.  In this case it is
** glPolygonStipple.
*/
#define __GLX_MAX_SMALL_RENDER_CMD_SIZE	156

/*
** To keep the implementation fast, the code uses a "limit" pointer
** to determine when the drawing command buffer is too full to hold
** another fixed size command.  This constant defines the amount of
** space that must always be available in the drawing command buffer
** at all times for the implementation to work.  It is important that
** the number be just large enough, but not so large as to reduce the
** efficacy of the buffer.  The "+32" is just to keep the code working
** in case somebody counts wrong.
*/
#define __GLX_BUFFER_LIMIT_SIZE	(__GLX_MAX_SMALL_RENDER_CMD_SIZE + 32)

/*
** This implementation uses a smaller threshold for switching
** to the RenderLarge protocol than the protcol requires so that
** large copies don't occur.
*/
#define __GLX_RENDER_CMD_SIZE_LIMIT	4096

/*
** One of these records exists per screen of the display.  It contains
** a pointer to the config data for that screen (if the screen supports GL).
*/
typedef struct __GLXscreenConfigsRec {
    __GLXvisualConfig *configs;
    int numConfigs;
    const char *serverGLXexts;
    char *effectiveGLXexts;
} __GLXscreenConfigs;

/*
** Per display private data.  One of these records exists for each display
** that is using the OpenGL (GLX) extension.
*/
struct __GLXdisplayPrivateRec {
    /*
    ** Back pointer to the display
    */
    Display *dpy;

    /*
    ** The majorOpcode is common to all connections to the same server.
    ** It is also copied into the context structure.
    */
    int majorOpcode;

    /*
    ** Major and minor version returned by the server during initialization.
    */
    int majorVersion, minorVersion;

    /* Storage for the servers GLX vendor and versions strings.  These
    ** are the same for all screens on this display. These fields will
    ** be filled in on demand.
    */
    char *serverGLXvendor;
    char *serverGLXversion;

    /*
    ** Configurations of visuals for all screens on this display.
    ** Also, per screen data which now includes the server GLX_EXTENSION
    ** string.
    */
    __GLXscreenConfigs *screenConfigs;

};

void __glXFreeContext(__GLXcontext*);

extern GLubyte *__glXFlushRenderBuffer(__GLXcontext*, GLubyte*);

extern void __glXSendLargeCommand(__GLXcontext *, const GLvoid *, GLint,
				  const GLvoid *, GLint);

/* Initialize the GLX extension for dpy */
extern __GLXdisplayPrivate *__glXInitialize(Display*);

/************************************************************************/

extern int __glXDebug;

/* This is per-thread storage in an MT environment */
extern __GLXcontext *__glXcurrentContext;
#define __glXGetCurrentContext()	__glXcurrentContext
#define __glXSetCurrentContext(gc)	__glXcurrentContext = gc

/*
** Global lock for all threads in this address space using the GLX
** extension
*/
#define __glXLock()
#define __glXUnlock()

/*
** Setup for a command.  Initialize the extension for dpy if necessary.
*/
extern CARD8 __glXSetupForCommand(Display *dpy);

/************************************************************************/

/*
** Data conversion and packing support.
*/

/* Return the size, in bytes, of some pixel data */
extern GLint __glImageSize(GLint, GLint, GLint, GLenum, GLenum);

/* Return the k value for a given map target */
extern GLint __glEvalComputeK(GLenum);

/*
** Fill the transport buffer with the data from the users buffer,
** applying some of the pixel store modes (unpack modes) to the data
** first.  As a side effect of this call, the "modes" field is
** updated to contain the modes needed by the server to decode the
** sent data.
*/
extern void __glFillImage(__GLXcontext*, GLint, GLint, GLint, GLint, GLenum,
			  GLenum, const GLvoid*, GLubyte*, GLubyte*);

/* Copy map data with a stride into a packed buffer */
extern void __glFillMap1f(GLint, GLint, GLint, const GLfloat *, GLubyte *);
extern void __glFillMap1d(GLint, GLint, GLint, const GLdouble *, GLubyte *);
extern void __glFillMap2f(GLint, GLint, GLint, GLint, GLint,
			  const GLfloat *, GLfloat *);
extern void __glFillMap2d(GLint, GLint, GLint, GLint, GLint,
			  const GLdouble *, GLdouble *);

/*
** Empty an image out of the reply buffer into the clients memory applying
** the pack modes to pack back into the clients requested format.
*/
extern void __glEmptyImage(__GLXcontext*, GLint, GLint, GLint, GLint, GLenum,
		           GLenum, const GLubyte *, GLvoid *);


/*
** Allocate and Initialize Vertex Array client state 
*/
extern void __glXInitVertexArrayState(__GLXcontext*);

/*
** Inform the Server of the major and minor numbers and of the client
** libraries extension string.
*/
extern void __glXClientInfo (  Display *dpy, int opcode );

/*
** Size routines.  These determine how much data needs to be transfered
** based on the clients arguments.  If an enumerant or other value
** is illegal these procedures return 0.
*/
extern GLint __glCallLists_size(GLint, GLenum);
extern GLint __glColorTableParameterfv_size(GLenum);
extern GLint __glColorTableParameteriv_size(GLenum);
extern GLint __glConvolutionParameterfv_size(GLenum);
extern GLint __glConvolutionParameteriv_size(GLenum);
extern GLint __glDrawPixels_size(GLenum, GLenum, GLint, GLint);
extern GLint __glReadPixels_size(GLenum, GLenum, GLint, GLint);
extern GLint __glLightModelfv_size(GLenum);
extern GLint __glLightModeliv_size(GLenum);
extern GLint __glLightfv_size(GLenum);
extern GLint __glLightiv_size(GLenum);
extern GLint __glMaterialfv_size(GLenum);
extern GLint __glMaterialiv_size(GLenum);
extern GLint __glFogfv_size(GLenum);
extern GLint __glFogiv_size(GLenum);
extern GLint __glTexImage1D_size(GLenum, GLenum, GLint);
extern GLint __glTexImage2D_size(GLenum, GLenum, GLint, GLint);
extern GLint __glTexImage3D_size(GLenum, GLenum, GLint, GLint, GLint);
extern GLint __glTexEnvfv_size(GLenum);
extern GLint __glTexEnviv_size(GLenum);
extern GLint __glTexGenfv_size(GLenum);
extern GLint __glTexGendv_size(GLenum);
extern GLint __glTexGeniv_size(GLenum);
extern GLint __glTexParameterfv_size(GLenum);
extern GLint __glTexParameteriv_size(GLenum);
extern GLint __glGetMaterialfv_size(GLenum);
extern GLint __glGetMaterialiv_size(GLenum);
extern GLint __glGetLightfv_size(GLenum);
extern GLint __glGetLightiv_size(GLenum);
extern GLint __glGetTexParameterfv_size(GLenum);
extern GLint __glGetTexParameteriv_size(GLenum);
extern GLint __glGetTexEnvfv_size(GLenum);
extern GLint __glGetTexEnviv_size(GLenum);
extern GLint __glGetTexGenfv_size(GLenum);
extern GLint __glGetTexGendv_size(GLenum);
extern GLint __glGetTexGeniv_size(GLenum);

/************************************************************************/

/*
** Declarations that should be in Xlib
*/
#ifdef __GL_USE_OUR_PROTOTYPES
extern void _XFlush(Display*);
extern Status _XReply(Display*, xReply*, int, Bool);
extern void _XRead(Display*, void*, long);
extern void _XSend(Display*, const void*, long);
#endif

#endif /* !__GLX_client_h__ */
