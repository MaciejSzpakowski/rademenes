#pragma once

namespace ph::gl
{
	void run(VOIDPROC action, VOIDPROC mouseMove, VOIDPROC keydown);
	void init();
	uint addSprite(float* color, float x, float y, float z, float w, float h);
	uint addSprite(spriteType type, float x, float y, float z, float w, float h);
	void updateSprite(uint index, float x, float y);
	void updateSprite(uint index, float x, float y, float z, float* color, float w, float h);
	void removeSprite(uint index);
	void error(const char* str);
	void setText(uint slot, const char* desc);
	bool isKeyDown(int key);
	bool isKeyPressed(int key);
	bool isKeyReleased(int key);
	void notify(const char* desc);
	void shuffleArray(byte* arr, uint len, uint elSize);
	/// <summary>
	/// inclusive min and exclusive max
	/// </summary>
	int rand(int min, int max);
	void _assert(bool cnd, const char* str);
	void clearSprites();
	void centerCamera();
}