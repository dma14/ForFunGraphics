#pragma once
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <dwrite.h>
#pragma comment(lib, "dwrite")
#include <string>
#include <iostream>
#include <cmath>
#include "BaseWindow.h"
#include "Utils.h"
#include "Scene.h"

constexpr auto fpsAverageFrames = 30;

class MainWindow : public BaseWindow<MainWindow>
{
public:
	// Required override functions
	PCWSTR ClassName() const { return L"Drawing Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Constructor to initialize class-specific variables
	MainWindow(){}

private:
	// Direct2D setup variables
	// Direct2D factory, always needed
	ID2D1Factory* pFactory = NULL;
	// DirectWrite factory, used for text
	IDWriteFactory* pWriteFactory = NULL;
	IDWriteTextFormat* pTextFormat = NULL;
	// Render target representing the window, so we can draw on it
	ID2D1HwndRenderTarget* pRenderTarget = NULL;
	// Brush resource, used for drawing
	ID2D1SolidColorBrush* pBrush = NULL;
	// Ellipse object, the one we're drawing
	D2D1_ELLIPSE ellipse;
	// Timer ID pointer
	const UINT_PTR timerId = 1;
	// Counter for time elapsed, in ms.
	TIME_MS_T startTime = 0;
	TIME_MS_T timeElapsed = 0;
	// Running average for cimputing FPS over time
	RunningAverage<TIME_MS_T> fpsAverage = RunningAverage<TIME_MS_T>(fpsAverageFrames);
	// Scene object
    Scene MyScene = Scene();

	// Function to create our Direct2D resources
	HRESULT CreateGraphicsResources();

	// Function to discard our Direct2D resources
	void DiscardGraphicsResources();

	// Function to handle the painting of our window
	void OnPaint();

	// Function to handle resizing of the window
	void Resize();

	// Function to update our own chapes
	void CalculateLayout();

	// Function to handle keypresses
    void HandleKeypress(WPARAM KeyCode);
};

