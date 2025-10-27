#pragma once
#include <bitset>
#include "SableUI/utils.h"

/* Borrowed from GLFW key codes for ease of use 
 * Printable Keys */
constexpr uint16_t SABLE_KEY_SPACE				= 32;
constexpr uint16_t SABLE_KEY_APOSTROPHE			= 39;  /* ' */
constexpr uint16_t SABLE_KEY_COMMA				= 44;  /* , */
constexpr uint16_t SABLE_KEY_MINUS				= 45;  /* - */
constexpr uint16_t SABLE_KEY_PERIOD				= 46;  /* . */
constexpr uint16_t SABLE_KEY_SLASH				= 47;  /* / */
constexpr uint16_t SABLE_KEY_0					= 48;
constexpr uint16_t SABLE_KEY_1					= 49;
constexpr uint16_t SABLE_KEY_2					= 50;
constexpr uint16_t SABLE_KEY_3					= 51;
constexpr uint16_t SABLE_KEY_4					= 52;
constexpr uint16_t SABLE_KEY_5					= 53;
constexpr uint16_t SABLE_KEY_6					= 54;
constexpr uint16_t SABLE_KEY_7					= 55;
constexpr uint16_t SABLE_KEY_8					= 56;
constexpr uint16_t SABLE_KEY_9					= 57;
constexpr uint16_t SABLE_KEY_SEMICOLON			= 59;  /* ; */
constexpr uint16_t SABLE_KEY_EQUAL				= 61;  /* = */
constexpr uint16_t SABLE_KEY_A					= 65;
constexpr uint16_t SABLE_KEY_B					= 66;
constexpr uint16_t SABLE_KEY_C					= 67;
constexpr uint16_t SABLE_KEY_D					= 68;
constexpr uint16_t SABLE_KEY_E					= 69;
constexpr uint16_t SABLE_KEY_F					= 70;
constexpr uint16_t SABLE_KEY_G					= 71;
constexpr uint16_t SABLE_KEY_H					= 72;
constexpr uint16_t SABLE_KEY_I					= 73;
constexpr uint16_t SABLE_KEY_J					= 74;
constexpr uint16_t SABLE_KEY_K					= 75;
constexpr uint16_t SABLE_KEY_L					= 76;
constexpr uint16_t SABLE_KEY_M					= 77;
constexpr uint16_t SABLE_KEY_N					= 78;
constexpr uint16_t SABLE_KEY_O					= 79;
constexpr uint16_t SABLE_KEY_P					= 80;
constexpr uint16_t SABLE_KEY_Q					= 81;
constexpr uint16_t SABLE_KEY_R					= 82;
constexpr uint16_t SABLE_KEY_S					= 83;
constexpr uint16_t SABLE_KEY_T					= 84;
constexpr uint16_t SABLE_KEY_U					= 85;
constexpr uint16_t SABLE_KEY_V					= 86;
constexpr uint16_t SABLE_KEY_W					= 87;
constexpr uint16_t SABLE_KEY_X					= 88;
constexpr uint16_t SABLE_KEY_Y					= 89;
constexpr uint16_t SABLE_KEY_Z					= 90;
constexpr uint16_t SABLE_KEY_LEFT_BRACKET		= 91;  /* [ */
constexpr uint16_t SABLE_KEY_BACKSLASH			= 92;  /* \ */
constexpr uint16_t SABLE_KEY_RIGHT_BRACKET		= 93;  /* ] */
constexpr uint16_t SABLE_KEY_GRAVE_ACCENT		= 96;  /* ` */

/* Function keys */
constexpr uint16_t SABLE_KEY_WORLD_1			= 161; /* non-US #1 */
constexpr uint16_t SABLE_KEY_WORLD_2			= 162; /* non-US #2 */
constexpr uint16_t SABLE_KEY_ESCAPE				= 256;
constexpr uint16_t SABLE_KEY_ENTER				= 257;
constexpr uint16_t SABLE_KEY_TAB				= 258;
constexpr uint16_t SABLE_KEY_BACKSPACE			= 259;
constexpr uint16_t SABLE_KEY_INSERT				= 260;
constexpr uint16_t SABLE_KEY_DELETE				= 261;
constexpr uint16_t SABLE_KEY_RIGHT				= 262;
constexpr uint16_t SABLE_KEY_LEFT				= 263;
constexpr uint16_t SABLE_KEY_DOWN				= 264;
constexpr uint16_t SABLE_KEY_UP					= 265;
constexpr uint16_t SABLE_KEY_PAGE_UP			= 266;
constexpr uint16_t SABLE_KEY_PAGE_DOWN			= 267;
constexpr uint16_t SABLE_KEY_HOME				= 268;
constexpr uint16_t SABLE_KEY_END				= 269;
constexpr uint16_t SABLE_KEY_CAPS_LOCK			= 280;
constexpr uint16_t SABLE_KEY_SCROLL_LOCK		= 281;
constexpr uint16_t SABLE_KEY_NUM_LOCK			= 282;
constexpr uint16_t SABLE_KEY_PRINT_SCREEN		= 283;
constexpr uint16_t SABLE_KEY_PAUSE				= 284;
constexpr uint16_t SABLE_KEY_F1					= 290;
constexpr uint16_t SABLE_KEY_F2					= 291;
constexpr uint16_t SABLE_KEY_F3					= 292;
constexpr uint16_t SABLE_KEY_F4					= 293;
constexpr uint16_t SABLE_KEY_F5					= 294;
constexpr uint16_t SABLE_KEY_F6					= 295;
constexpr uint16_t SABLE_KEY_F7					= 296;
constexpr uint16_t SABLE_KEY_F8					= 297;
constexpr uint16_t SABLE_KEY_F9					= 298;
constexpr uint16_t SABLE_KEY_F10				= 299;
constexpr uint16_t SABLE_KEY_F11				= 300;
constexpr uint16_t SABLE_KEY_F12				= 301;
constexpr uint16_t SABLE_KEY_F13				= 302;
constexpr uint16_t SABLE_KEY_F14				= 303;
constexpr uint16_t SABLE_KEY_F15				= 304;
constexpr uint16_t SABLE_KEY_F16				= 305;
constexpr uint16_t SABLE_KEY_F17				= 306;
constexpr uint16_t SABLE_KEY_F18				= 307;
constexpr uint16_t SABLE_KEY_F19				= 308;
constexpr uint16_t SABLE_KEY_F20				= 309;
constexpr uint16_t SABLE_KEY_F21				= 310;
constexpr uint16_t SABLE_KEY_F22				= 311;
constexpr uint16_t SABLE_KEY_F23				= 312;
constexpr uint16_t SABLE_KEY_F24				= 313;
constexpr uint16_t SABLE_KEY_F25				= 314;
constexpr uint16_t SABLE_KEY_KP_0				= 320;
constexpr uint16_t SABLE_KEY_KP_1				= 321;
constexpr uint16_t SABLE_KEY_KP_2				= 322;
constexpr uint16_t SABLE_KEY_KP_3				= 323;
constexpr uint16_t SABLE_KEY_KP_4				= 324;
constexpr uint16_t SABLE_KEY_KP_5				= 325;
constexpr uint16_t SABLE_KEY_KP_6				= 326;
constexpr uint16_t SABLE_KEY_KP_7				= 327;
constexpr uint16_t SABLE_KEY_KP_8				= 328;
constexpr uint16_t SABLE_KEY_KP_9				= 329;
constexpr uint16_t SABLE_KEY_KP_DECIMAL			= 330;
constexpr uint16_t SABLE_KEY_KP_DIVIDE			= 331;
constexpr uint16_t SABLE_KEY_KP_MULTIPLY		= 332;
constexpr uint16_t SABLE_KEY_KP_SUBTRACT		= 333;
constexpr uint16_t SABLE_KEY_KP_ADD				= 334;
constexpr uint16_t SABLE_KEY_KP_ENTER			= 335;
constexpr uint16_t SABLE_KEY_KP_EQUAL			= 336;
constexpr uint16_t SABLE_KEY_LEFT_SHIFT			= 340;
constexpr uint16_t SABLE_KEY_LEFT_CONTROL		= 341;
constexpr uint16_t SABLE_KEY_LEFT_ALT			= 342;
constexpr uint16_t SABLE_KEY_LEFT_SUPER			= 343;
constexpr uint16_t SABLE_KEY_RIGHT_SHIFT		= 344;
constexpr uint16_t SABLE_KEY_RIGHT_CONTROL		= 345;
constexpr uint16_t SABLE_KEY_RIGHT_ALT			= 346;
constexpr uint16_t SABLE_KEY_RIGHT_SUPER		= 347;
constexpr uint16_t SABLE_KEY_MENU				= 348;

/* Mouse buttons */
constexpr uint16_t SABLE_MOUSE_BUTTON_1			= 0;
constexpr uint16_t SABLE_MOUSE_BUTTON_2			= 1;
constexpr uint16_t SABLE_MOUSE_BUTTON_3			= 2;
constexpr uint16_t SABLE_MOUSE_BUTTON_4			= 3;
constexpr uint16_t SABLE_MOUSE_BUTTON_5			= 4;
constexpr uint16_t SABLE_MOUSE_BUTTON_6			= 5;
constexpr uint16_t SABLE_MOUSE_BUTTON_7			= 6;
constexpr uint16_t SABLE_MOUSE_BUTTON_8			= 7;
constexpr uint16_t SABLE_MOUSE_BUTTON_LAST		= SABLE_MOUSE_BUTTON_8;
constexpr uint16_t SABLE_MOUSE_BUTTON_LEFT		= SABLE_MOUSE_BUTTON_1;
constexpr uint16_t SABLE_MOUSE_BUTTON_RIGHT		= SABLE_MOUSE_BUTTON_2;
constexpr uint16_t SABLE_MOUSE_BUTTON_MIDDLE	= SABLE_MOUSE_BUTTON_3;

constexpr uint16_t SABLE_MAX_KEYS				= SABLE_KEY_MENU + 1;
constexpr uint16_t SABLE_MIN_PRINTABLE_KEY		= SABLE_KEY_SPACE;
constexpr uint16_t SABLE_MAX_PRINTABLE_KEY		= SABLE_KEY_GRAVE_ACCENT;
constexpr uint16_t SABLE_MAX_MOUSE_BUTTONS		= SABLE_MOUSE_BUTTON_8 + 1;

constexpr bool SABLE_MOUSE_DOWN = true;
constexpr bool SABLE_MOUSE_UP = false;

namespace SableUI
{
	struct UIEventContext
	{
		float deltaTime = 0.0f;

		ivec2 mousePos = { 0, 0 };
		ivec2 mouseDelta = { 0, 0 };
		float scrollDelta = 0.0f;

		std::bitset<SABLE_MAX_MOUSE_BUTTONS> mouseDown;
		std::bitset<SABLE_MAX_MOUSE_BUTTONS> mousePressed;
		std::bitset<SABLE_MAX_MOUSE_BUTTONS> mouseReleased;
		std::bitset<SABLE_MAX_MOUSE_BUTTONS> mouseDoubleClicked;

		std::bitset<SABLE_MAX_KEYS> keyDown;
		std::bitset<SABLE_MAX_KEYS> keyPressed;
		std::bitset<SABLE_MAX_KEYS> keyReleased;
	};

	inline bool IsMouseDown(const UIEventContext& ctx, uint8_t button) { return ctx.mouseDown.test(button); }
	inline bool IsMousePressed(const UIEventContext& ctx, uint8_t button) { return ctx.mousePressed.test(button); }
	inline bool IsMouseReleased(const UIEventContext& ctx, uint8_t button) { return ctx.mouseReleased.test(button); }
}