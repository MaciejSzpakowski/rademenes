#pragma once

namespace ph::builder
{
	void leftClickCallback();
	void rightClickCallback();
	void select(int delta);
	const char* getDesc();
	void keyboard();
	void init();
}