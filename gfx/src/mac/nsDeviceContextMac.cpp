/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsDeviceContextMac.h"
#include "nsRenderingContextMac.h"
//XXX#include "../nsGfxCIID.h"

#include "math.h"
#include "nspr.h"
#include "il_util.h"
#include <FixMath.h>
#include <Fonts.h>

static NS_DEFINE_IID(kDeviceContextIID, NS_IDEVICE_CONTEXT_IID);

//------------------------------------------------------------------------

nsDeviceContextMac :: nsDeviceContextMac()
{
GDHandle			thegd;
PixMapHandle	thepix;
double				pix_inch;

  NS_INIT_REFCNT();
  
  // see IM Imaging with Quickdraw, chapter 5.  This is an incomplete implementation
  
  // cps - see technote <http://developer.apple.com/technotes/tn/tn1118.html>
  // Basically it says: don't unlock GDevice handles
  thegd = ::GetMainDevice();
  SInt8 hState = ::HGetState ((Handle) thegd);
  ::HLock((Handle)thegd);
	thepix = (**thegd).gdPMap;
	
    // Be sure to lock the PixMapHandle before dereferencing it
    SInt8 PixMapHState = ::HGetState ((Handle) thepix);
    ::HLock((Handle)thepix);
	pix_inch = Fix2X((**thepix).hRes);
	
	mTwipsToPixels = pix_inch/(float)NSIntPointsToTwips(72);
	mPixelsToTwips = 1.0f/mTwipsToPixels;
	
	mDepth = (**thepix).pixelSize;
	
    ::HSetState ((Handle)thepix,PixMapHState);
  // cps - Unlocking GDeviceHandles is a no - no. See above.
  //::HUnlock((Handle)thegd);
  ::HSetState ((Handle)thegd,hState);
}

//------------------------------------------------------------------------

nsDeviceContextMac :: ~nsDeviceContextMac()
{
}

//------------------------------------------------------------------------

NS_IMPL_QUERY_INTERFACE(nsDeviceContextMac, kDeviceContextIID)
NS_IMPL_ADDREF(nsDeviceContextMac)
NS_IMPL_RELEASE(nsDeviceContextMac)

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: Init(nsNativeWidget aNativeWidget)
{
  NS_ASSERTION(!(aNativeWidget == nsnull), "attempt to init devicecontext with null widget");


	// this is a windowptr, or grafptr, native to macintosh only
	mSurface = aNativeWidget;

  return NS_OK;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: CreateRenderingContext(nsIRenderingContext *&aContext)
{
  return NS_ERROR_FAILURE;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: SupportsNativeWidgets(PRBool &aSupportsWidgets)
{
  //XXX it is very critical that this not lie!! MMP
  aSupportsWidgets = PR_FALSE;

  return NS_OK;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: GetScrollBarDimensions(float &aWidth, float &aHeight) const
{
  // XXX Should we push this to widget library
  aWidth = 320.0;
  aHeight = 320.0;
  return NS_OK;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: GetDrawingSurface(nsIRenderingContext &aContext, nsDrawingSurface &aSurface)
{
  aContext.CreateDrawingSurface(nsnull, 0, aSurface);
  return nsnull == aSurface ? NS_ERROR_OUT_OF_MEMORY : NS_OK;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::GetDepth(PRUint32& aDepth)
{
  aDepth = mDepth;
  return NS_OK;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::CreateILColorSpace(IL_ColorSpace*& aColorSpace)
{
  nsresult result = NS_OK;


  return result;
}

//------------------------------------------------------------------------


NS_IMETHODIMP nsDeviceContextMac::GetILColorSpace(IL_ColorSpace*& aColorSpace)
{

  if (nsnull == mColorSpace) {
    IL_RGBBits colorRGBBits;
  
    // Default is to create a 32-bit color space
    colorRGBBits.red_shift = 16;  
    colorRGBBits.red_bits = 8;
    colorRGBBits.green_shift = 8;
    colorRGBBits.green_bits = 8; 
    colorRGBBits.blue_shift = 0; 
    colorRGBBits.blue_bits = 8;  
  
    mColorSpace = IL_CreateTrueColorSpace(&colorRGBBits, 32);
    if (nsnull == mColorSpace) {
      aColorSpace = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  NS_POSTCONDITION(nsnull != mColorSpace, "null color space");
  aColorSpace = mColorSpace;
  IL_AddRefToColorSpace(aColorSpace);
  return NS_OK;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: CheckFontExistence(const nsString& aFontName)
{
  	short fontNum;
	if (GetMacFontNumber(aFontName, fontNum))
		return NS_OK;
	else
		return NS_ERROR_FAILURE;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::GetDeviceSurfaceDimensions(PRInt32 &aWidth, PRInt32 &aHeight)
{
  aWidth = 1;
  aHeight = 1;

  return NS_ERROR_FAILURE;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::GetDeviceContextFor(nsIDeviceContextSpec *aDevice,
                                                      nsIDeviceContext *&aContext)
{
  return NS_ERROR_FAILURE;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::BeginDocument(void)
{
  return NS_OK;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::EndDocument(void)
{
  return NS_OK;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::BeginPage(void)
{
  return NS_OK;
}


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::EndPage(void)
{
  return NS_OK;
}


//------------------------------------------------------------------------

bool nsDeviceContextMac :: GetMacFontNumber(const nsString& aFontName, short &aFontNum)
{
	Str255 		systemFontName;
	Str255		aStr;
	bool			fontExists;

	//�TODO?: Maybe we shouldn't call that function so often. If nsFont could store the
	//				fontNum, nsFontMetricsMac::SetFont() wouldn't need to call this at all.
	static nsString		lastFontName;
	static short			lastFontNum;
	static bool				lastFontExists;

	if (lastFontName == aFontName)
	{
		aFontNum = lastFontNum;
		return lastFontExists;
	}

	aStr[0] = aFontName.Length();
	aFontName.ToCString((char*)&aStr[1], sizeof(aStr)-1);

	::GetFNum(aStr, &aFontNum);
	if (aFontNum == 0)
	{
		// Either we didn't find the font, or we were looking for the system font
		::GetFontName(0, systemFontName);
		fontExists = ::EqualString(aStr, systemFontName, FALSE, FALSE );
	}
	else
		fontExists = true;

	lastFontExists = fontExists;
	lastFontNum = aFontNum;
	lastFontName = aFontName;

	return fontExists;
}

//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac :: ConvertPixel(nscolor aColor, PRUint32 & aPixel)
{
  aPixel = aColor;
  return NS_OK;
}

