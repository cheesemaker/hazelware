#include "FWInclude.h"
#include "MMInclude.h"

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class Complex
{
	public:
		Complex() : real(0), imag(0) {;}
		Complex(long double a, long double b) : real(a), imag(b) {;}

		Complex operator + (const Complex& c2) { return Complex(c2.real+real,c2.imag+imag);	};
		Complex operator - (const Complex& c2) { return Complex(real-c2.real,imag-c2.imag);	};
		Complex operator * (const Complex& c2) { return Complex(real*c2.real-imag*c2.imag,real*c2.imag+c2.real*imag); };

		bool Escaped() const {	return ((real * real + imag * imag) >= 4.0); }
		void Plot(void* pixel, MMBitmapFormat format, unsigned long iterations);
		long double real;
		long double imag;
};


void Complex::Plot(void* rawPixel, MMBitmapFormat format, unsigned long iterations)
{
	Complex c(real, imag);
	Complex a(real, imag);

	unsigned long i = 0;
	bool escaped = false;
	while (i++ < iterations && !escaped)
	{
		a = a * a + c;
		escaped = a.Escaped();
	}
	
	unsigned char blue = (!escaped) ? (unsigned char)0 : (unsigned char)(255-(i*10));
	MMSetPixel(rawPixel, MMSystemColorType(0, 0, blue), format);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class Mandelbrot : public MMWidget
{
	public:
		Mandelbrot(const FWRect& area) : 
			MMWidget(area, 0), 
			left(-2.0), top(-2.0), right(2.0), bottom(2.0),
			itsIsInvalid(true)
		{
			itsCanvas.Set(NEW(MMRGBSystemCanvas(area.GetWidth(), area.GetHeight())));
		}

		bool HandleMessage(const FWPointer<MMMessage>& message)
		{ 
			if (message == "Move left")
			{
				MoveLeft();
			}
			if (message == "Move right")
			{
				MoveRight();
			}
			if (message == "Move up")
			{
				MoveUp();
			}
			if (message == "Move down")
			{
				MoveDown();
			}
			if (message == "Reset")
			{
				Reset();
			}
			if (message == "Zoom in")
			{
				ZoomIn();
			}
			if (message == "Zoom out")
			{
				ZoomOut();
			}

			return false;
		}

		bool IsFocusable() const { return false; }

		void Draw(const FWPointer<MMCanvas>& canvas,const FWRect& /*area*/)
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			MMBitmap* sourceBitmap = itsCanvas->LockBitmap();
			MMBitmap* bitmap = canvas->LockBitmap();
			MMBitmapCopier copier(bitmap);
			copier.CopyFrom(sourceBitmap, FWRect(0, 0, (int)sourceBitmap->GetWidth(), (int)sourceBitmap->GetHeight()), GetPosition());
			itsCanvas->UnlockBitmap();
			canvas->UnlockBitmap();
		}
		
		void DrawLine(unsigned long y, unsigned long iterations)
		{
			FWCriticalSectionScope scope(&itsCriticalSection);
			FWRect bounds = GetBounds();
			if (y >= bounds.GetHeight())
				return;
				
			MMBitmap* bitmap = itsCanvas->LockBitmap();
			
			long double xInc = ((right - left) / bounds.GetWidth());
			long double yInc = ((bottom - top) / bounds.GetHeight());
			long double xReal = left;
			long double yReal = top;
			unsigned long height = bounds.GetHeight();
			unsigned long width = bounds.GetWidth();
			Complex complex(xReal, yReal);
			complex.imag = complex.imag + (yInc * y);
			for (unsigned long x = 0; x < width; x++)
			{
				complex.Plot(bitmap->GetPixel(FWPoint((int)x, (int)y)), bitmap->GetFormat(), iterations);
				complex.real += xInc;
			}
			itsCanvas->UnlockBitmap();
		}

		void Reset()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			left = -2.0;
			top = -2.0;
			right = 2.0;
			bottom = 2.0;
			Invalidate();
			SetInvalidationStatus(true);
		}

		void MoveLeft()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double x = ((right - left) / 10.0);
			left = left - x;
			right = right - x;
			Invalidate();
			SetInvalidationStatus(true);
		}

		void MoveRight()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double x = ((right - left) / 10.0);
			left = left + x;
			right = right + x;
			Invalidate();
			SetInvalidationStatus(true);
		}

		void MoveUp()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double y = ((bottom - top) / 10.0);
			top = top - y;
			bottom = bottom - y;
			Invalidate();
			SetInvalidationStatus(true);
		}
		
		void MoveDown()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double y = ((bottom - top) / 10.0);
			top = top + y;
			bottom = bottom + y;
			Invalidate();
			SetInvalidationStatus(true);
		}
		
		void ZoomIn()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double width = right - left;
			long double height = bottom - top;

			long double x = left + ((right - left) / 2.0);
			long double y = top + ((bottom - top) / 2.0);

			width = (width * 0.9) / 2.0;
			height = (height * 0.9) / 2.0;

			left = x - width;
			top = y - height;
			right = x + width;
			bottom = y + height;
			Invalidate();
			SetInvalidationStatus(true);
		}

		void ZoomOut()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);

			long double width = right - left;
			long double height = bottom - top;

			long double x = left + ((right - left) / 2.0);
			long double y = top + ((bottom - top) / 2.0);

			width = (width * 1.1) / 2.0;
			height = (height * 1.1) / 2.0;

			left = x - width;
			top = y - height;
			right = x + width;
			bottom = y + height;
			Invalidate();
			SetInvalidationStatus(true);
		}

		bool IsMandelInvalid()
		{
			FWCriticalSectionScope scope(&itsCriticalSection);
			return itsIsInvalid;
		}
		
		void SetInvalidationStatus(bool invalid)
		{
			FWCriticalSectionScope scope(&itsCriticalSection);
			itsIsInvalid = invalid;
		}

		long double left;
		long double top;
		long double right;
		long double bottom;
		bool		itsIsInvalid;

		FWPointer<MMRGBSystemCanvas>	itsCanvas;
		FWCriticalSection				itsCriticalSection;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class MandleThread : public FWThread
{
	public:
		MandleThread(Mandelbrot* mandelbrot) : 
			itsMandelbrot(mandelbrot),
			itsMax(mandelbrot->GetBounds().GetHeight()),
			itsCurrent(0),
			itsIterations(30)
		{
		}
		
		void Execute()
		{
			if (itsMandelbrot->IsMandelInvalid())
			{
				itsMandelbrot->SetInvalidationStatus(false);
				itsCurrent = 0;
			}
			
			if (itsCurrent < itsMax)
			{
				itsMandelbrot->DrawLine(itsCurrent, itsIterations);
				itsCurrent++;
				
/*				itsMandelbrot->DrawLine(itsCurrent, itsIterations);
				itsCurrent++;
				
				itsMandelbrot->DrawLine(itsCurrent, itsIterations);
				itsCurrent++;
				
				itsMandelbrot->DrawLine(itsCurrent, itsIterations);
				itsCurrent++;

*/				itsMandelbrot->Invalidate();
			}
		}
		
		Mandelbrot* itsMandelbrot;
		unsigned long itsCurrent;
		unsigned long itsMax;
		unsigned long itsIterations;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class MyApplication : public MMApplication
{
	///////////////////////////////////////////////////////////////////////
	//Provide information about the main window that you want to create...
	FWWindowStyle GetMainWindowInfo()
	{
		FWWindowStyle style(FWStr("Hazelware Demo"));
		style.SetStyles(0, 0);
		return style;
	}

	////////////////////////////////////////////////////////////////////
	//OnAppStartup is our first chance to actually 'DO' something.  
	//Usually, we just set up the first form here.
	void OnAppStartup()
	{
		//Create the custom Mandlebrot object...
		FWRect rect(49, 1, (int)UUSystem::GetScreenWidth() - 4, (int)UUSystem::GetScreenHeight() - 52);
		itsForm = NEW(MMForm);
		Mandelbrot* mandelbrot = NEW(Mandelbrot(rect));
		itsForm->Add(mandelbrot);
	
		//Create a green border around the Mandelbrot object...
		rect.Inset(-1);
		itsForm->Add(NEW(MMFrameWidget(rect, MMSystemColorType(0,170,0), 0)));

		//Set our form as the active form...
		SetActiveForm(itsForm);

		//Add the buttons to do stuff...
		itsForm->Add(NEW(MMGrayButton(FWStr("Zoom+"), FWRect(2, 1, 45, 18), 50, MMMessageString("Zoom in"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Zoom-"), FWRect(2, 20, 45, 38), 50, MMMessageString("Zoom out"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Left"), FWRect(2, 40, 45, 58), 50, MMMessageString("Move left"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Right"), FWRect(2, 60, 45, 78), 50, MMMessageString("Move right"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Up"), FWRect(2, 80, 45, 98), 50, MMMessageString("Move up"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Down"), FWRect(2, 100, 45, 118), 50, MMMessageString("Move down"))));
		itsForm->Add(NEW(MMGrayButton(FWStr("Reset"), FWRect(2, 120, 45, 138), 50, MMMessageString("Reset"))));
		
		MandleThread* thread = NEW(MandleThread(mandelbrot));
		thread->Start();
	}


	////////////////////////////////////////////////////////////////////
	// HandleMessage is where stuff happens. Check the message input to
	// respond to different things...
	void HandleMessage(const FWPointer<MMMessage>& /*message*/)
	{
	}

	private:
		MMForm*		itsForm;
};



////////////////////////////////////////////////////////////////////////////////
// This is the default framework 'entry-point' 
// It is here because I'm not sure where else to put it...
FWApplication* FWMain()
{
	return NEW(MyApplication);
}