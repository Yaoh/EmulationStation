#include "guis/GuiInputConfig.h"

#include "components/ButtonComponent.h"
#include "components/MenuComponent.h"
#include "guis/GuiMsgBox.h"
#include "InputManager.h"
#include "Log.h"
#include "Window.h"
#include "Locale.h"

// static const int inputCount = 10;
// static const char* inputName[inputCount] = { "Up", "Down", "Left", "Right", "A", "B", "Start", "Select", "PageUp", "PageDown" };
// static const bool inputSkippable[inputCount] = { false, false, false, false, false, false, false, false, true, true };
// static const char* inputDispName[inputCount] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "START", "SELECT", "PAGE UP", "PAGE DOWN" };
// static const char* inputIcon[inputCount] = { ":/help/dpad_up.svg", ":/help/dpad_down.svg", ":/help/dpad_left.svg", ":/help/dpad_right.svg", 
// 											":/help/button_a.svg", ":/help/button_b.svg", ":/help/button_start.svg", ":/help/button_select.svg", 
// 											":/help/button_l.svg", ":/help/button_r.svg" };

static const int inputCount = 25;
static const char* inputName[inputCount] =
{
	"Up",
	"Down",
	"Left",
	"Right",
	"Start",
	"Select",
	"A",
	"B",
	"X",
	"Y",
	"LeftShoulder",
	"RightShoulder",
	"LeftTrigger",
	"RightTrigger",
	"LeftThumb",
	"RightThumb",
	"LeftAnalogUp",
	"LeftAnalogDown",
	"LeftAnalogLeft",
	"LeftAnalogRight",
	"RightAnalogUp",
	"RightAnalogDown",
	"RightAnalogLeft",
	"RightAnalogRight",
	"HotKeyEnable"
};
static const bool inputSkippable[inputCount] =
{
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true
};

#define gettext_noop(s) s

static const char *inputDispName[inputCount] =
{
	gettext_noop("D-PAD UP"),
	gettext_noop("D-PAD DOWN"),
	gettext_noop("D-PAD LEFT"),
	gettext_noop("D-PAD RIGHT"),
	gettext_noop("START"),
	gettext_noop("SELECT_BTN"),
	gettext_noop("A"),
	gettext_noop("B"),
	gettext_noop("X"),
	gettext_noop("Y"),
	gettext_noop("LEFT SHOULDER"),
	gettext_noop("RIGHT SHOULDER"),
	gettext_noop("LEFT TRIGGER"),
	gettext_noop("RIGHT TRIGGER"),
	gettext_noop("LEFT THUMB"),
	gettext_noop("RIGHT THUMB"),
	gettext_noop("LEFT ANALOG UP"),
	gettext_noop("LEFT ANALOG DOWN"),
	gettext_noop("LEFT ANALOG LEFT"),
	gettext_noop("LEFT ANALOG RIGHT"),
	gettext_noop("RIGHT ANALOG UP"),
	gettext_noop("RIGHT ANALOG DOWN"),
	gettext_noop("RIGHT ANALOG LEFT"),
	gettext_noop("RIGHT ANALOG RIGHT"),
	gettext_noop("HOTKEY ENABLE")
};
static const char* inputIcon[inputCount] =
{
	":/help/dpad_up.svg",
	":/help/dpad_down.svg",
	":/help/dpad_left.svg",
	":/help/dpad_right.svg",
	":/help/button_start.svg",
	":/help/button_select.svg",
	":/help/button_a.svg",
	":/help/button_b.svg",
	":/help/button_x.svg",
	":/help/button_y.svg",
	":/help/button_l.svg",
	":/help/button_r.svg",
	":/help/button_l.svg",
	":/help/button_r.svg",
	":/help/analog_thumb.svg",
	":/help/analog_thumb.svg",
	":/help/analog_up.svg",
	":/help/analog_down.svg",
	":/help/analog_left.svg",
	":/help/analog_right.svg",
	":/help/analog_up.svg",
	":/help/analog_down.svg",
	":/help/analog_left.svg",
	":/help/analog_right.svg",
	":/help/button_hotkey.svg"
};

//MasterVolUp and MasterVolDown are also hooked up, but do not appear on this screen.
//If you want, you can manually add them to es_input.cfg.

#define HOLD_TO_SKIP_MS 1000

GuiInputConfig::GuiInputConfig(Window* window, InputConfig* target, bool reconfigureAll, const std::function<void()>& okCallback) : GuiComponent(window), 
	mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 7)), 
	mTargetConfig(target), mHoldingInput(false), mBusyAnim(window)
{
	LOG(LogInfo) << "Configuring device " << target->getDeviceId() << " (" << target->getDeviceName() << ").";

	if(reconfigureAll)
		target->clear();

	mConfiguringAll = reconfigureAll;
	mConfiguringRow = mConfiguringAll;

	addChild(&mBackground);
	addChild(&mGrid);

	// 0 is a spacer row
	mGrid.setEntry(std::make_shared<GuiComponent>(mWindow), Vector2i(0, 0), false);

	mTitle = std::make_shared<TextComponent>(mWindow, _("CONFIGURING"), Font::get(FONT_SIZE_LARGE), 0x555555FF, ALIGN_CENTER);
	mGrid.setEntry(mTitle, Vector2i(0, 1), false, true);
	
	std::string deviceName;
	if(target->getDeviceId() == DEVICE_KEYBOARD)
		deviceName = _("KEYBOARD");
	else if(target->getDeviceId() == DEVICE_CEC)
		deviceName = _("CEC");
	else
		deviceName = (boost::locale::format(_("GAMEPAD {1}")) % (target->getDeviceId() + 1)).str();
	mSubtitle1 = std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(deviceName), Font::get(FONT_SIZE_MEDIUM), 0x555555FF, ALIGN_CENTER);
	mGrid.setEntry(mSubtitle1, Vector2i(0, 2), false, true);

	mSubtitle2 = std::make_shared<TextComponent>(mWindow, _("HOLD ANY BUTTON TO SKIP"), Font::get(FONT_SIZE_SMALL), 0x99999900, ALIGN_CENTER);
	mGrid.setEntry(mSubtitle2, Vector2i(0, 3), false, true);

	// 4 is a spacer row

	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 5), true, true);
	for(int i = 0; i < inputCount; i++)
	{
		ComponentListRow row;
		
		// icon
		auto icon = std::make_shared<ImageComponent>(mWindow);
		icon->setImage(inputIcon[i]);
		icon->setColorShift(0x777777FF);
		icon->setResize(0, Font::get(FONT_SIZE_MEDIUM)->getLetterHeight() * 1.25f);
		row.addElement(icon, false);

		// spacer between icon and text
		auto spacer = std::make_shared<GuiComponent>(mWindow);
		spacer->setSize(16, 0);
		row.addElement(spacer, false);

		auto text = std::make_shared<TextComponent>(mWindow, _(inputDispName[i]), Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		row.addElement(text, true);

		auto mapping = std::make_shared<TextComponent>(mWindow, _("-NOT DEFINED-"), Font::get(FONT_SIZE_MEDIUM, FONT_PATH_LIGHT), 0x999999FF, ALIGN_RIGHT);
		setNotDefined(mapping); // overrides text and color set above
		row.addElement(mapping, true);
		mMappings.push_back(mapping);

		row.input_handler = [this, i, mapping](InputConfig* config, Input input) -> bool
		{
			// ignore input not from our target device
			if(config != mTargetConfig)
				return false;

			// if we're not configuring, start configuring when A is pressed
			if(!mConfiguringRow)
			{
				if(config->isMappedTo("a", input) && input.value)
				{
					mList->stopScrolling();
					mConfiguringRow = true;
					setPress(mapping);
					return true;
				}
				
				// we're not configuring and they didn't press A to start, so ignore this
				return false;
			}

			// we are configuring
			if(input.value != 0)
			{
				// input down
				// if we're already holding something, ignore this, otherwise plan to map this input
				if(mHoldingInput)
					return true;

				mHoldingInput = true;
				mHeldInput = input;
				mHeldTime = 0;
				mHeldInputId = i;

				return true;
			}else{
				// input up
				// make sure we were holding something and we let go of what we were previously holding
				if(!mHoldingInput || mHeldInput.device != input.device || mHeldInput.id != input.id || mHeldInput.type != input.type)
					return true;

				mHoldingInput = false;

				if(assign(mHeldInput, i))
					rowDone(); // if successful, move cursor/stop configuring - if not, we'll just try again

				return true;
			}
		};

		mList->addRow(row);
	}

	// only show "HOLD TO SKIP" if this input is skippable
	mList->setCursorChangedCallback([this](CursorState /*state*/) {
		bool skippable = inputSkippable[mList->getCursorId()];
		mSubtitle2->setOpacity(skippable * 255);
	});

	// make the first one say "PRESS ANYTHING" if we're re-configuring everything
	if(mConfiguringAll)
		setPress(mMappings.front());

	// buttons
	std::vector< std::shared_ptr<ButtonComponent> > buttons;
	std::function<void()> okFunction = [this, okCallback] {
		InputManager::getInstance()->writeDeviceConfig(mTargetConfig); // save
		if(okCallback)
			okCallback();
		delete this; 
	};
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, _("OK"), _("OK"), [this, okFunction] {
		// check if the hotkey enable button is set. if not prompt the user to use select or nothing.
		Input input;
		if (!mTargetConfig->getInputByName("HotKeyEnable", &input)) {
			mWindow->pushGui(new GuiMsgBox(mWindow,
				_("YOU DIDN'T CHOOSE A HOTKEY ENABLE BUTTON. THIS IS REQUIRED FOR EXITING GAMES WITH A CONTROLLER. DO YOU WANT TO USE THE SELECT BUTTON DEFAULT ? PLEASE ANSWER YES TO USE SELECT OR NO TO NOT SET A HOTKEY ENABLE BUTTON."),
				_("YES"), [this, okFunction] {
					Input input;
					mTargetConfig->getInputByName("Select", &input);
					mTargetConfig->mapInput("HotKeyEnable", input);
					okFunction();
					},
				_("NO"), [this, okFunction] {
					// for a disabled hotkey enable button, set to a key with id 0,
					// so the input configuration script can be backwards compatible.
					mTargetConfig->mapInput("HotKeyEnable", Input(DEVICE_KEYBOARD, TYPE_KEY, 0, 1, true));
					okFunction();
				}
			));
		} else {
			okFunction();
		}
	}));
	mButtonGrid = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 6), true, false);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.75f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiInputConfig::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// update grid
	mGrid.setSize(mSize);

	//mGrid.setRowHeightPerc(0, 0.025f);
	mGrid.setRowHeightPerc(1, mTitle->getFont()->getHeight()*0.75f / mSize.y());
	mGrid.setRowHeightPerc(2, mSubtitle1->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(3, mSubtitle2->getFont()->getHeight() / mSize.y());
	//mGrid.setRowHeightPerc(4, 0.03f);
	mGrid.setRowHeightPerc(5, (mList->getRowHeight(0) * 5 + 2) / mSize.y());
	mGrid.setRowHeightPerc(6, mButtonGrid->getSize().y() / mSize.y());

	mBusyAnim.setSize(mSize);
}

void GuiInputConfig::update(int deltaTime)
{
	if(mConfiguringRow && mHoldingInput && inputSkippable[mHeldInputId])
	{
		int prevSec = mHeldTime / 1000;
		mHeldTime += deltaTime;
		int curSec = mHeldTime / 1000;

		if(mHeldTime >= HOLD_TO_SKIP_MS)
		{
			setNotDefined(mMappings.at(mHeldInputId));
			clearAssignment(mHeldInputId);
			mHoldingInput = false;
			rowDone();
		}else{
			if(prevSec != curSec)
			{
				// crossed the second boundary, update text
				const auto& text = mMappings.at(mHeldInputId);
				const int delay =  HOLD_TO_SKIP_MS/1000 - curSec;
				std::string message = (boost::locale::format(ngettext("HOLD FOR {1}S TO SKIP", "HOLD FOR {1}S TO SKIP", delay)) % delay).str();
				text->setText(message);
				text->setColor(0x777777FF);
			}
		}
	}
}

// move cursor to the next thing if we're configuring all, 
// or come out of "configure mode" if we were only configuring one row
void GuiInputConfig::rowDone()
{
	if(mConfiguringAll)
	{
		if(!mList->moveCursor(1)) // try to move to the next one
		{
			// at bottom of list, done
			mConfiguringAll = false;
			mConfiguringRow = false;
			mGrid.moveCursor(Vector2i(0, 1));
		}else{
			// on another one
			setPress(mMappings.at(mList->getCursorId()));
		}
	}else{
		// only configuring one row, so stop
		mConfiguringRow = false;
	}
}

void GuiInputConfig::setPress(const std::shared_ptr<TextComponent>& text)
{
	text->setText(_("PRESS ANYTHING"));
	text->setColor(0x656565FF);
}

void GuiInputConfig::setNotDefined(const std::shared_ptr<TextComponent>& text)
{
	text->setText(_("-NOT DEFINED-"));
	text->setColor(0x999999FF);
}

void GuiInputConfig::setAssignedTo(const std::shared_ptr<TextComponent>& text, Input input)
{
	text->setText(Utils::String::toUpper(input.string()));
	text->setColor(0x777777FF);
}

void GuiInputConfig::error(const std::shared_ptr<TextComponent>& text, const std::string& /*msg*/)
{
	text->setText(_("ALREADY TAKEN"));
	text->setColor(0x656565FF);
}

bool GuiInputConfig::assign(Input input, int inputId)
{
	// input is from InputConfig* mTargetConfig

	// if this input is mapped to something other than "nothing" or the current row, error
	// (if it's the same as what it was before, allow it)
	if(mTargetConfig->getMappedTo(input).size() > 0 && !mTargetConfig->isMappedTo(inputName[inputId], input) && strcmp(inputName[inputId], "HotKeyEnable") != 0)
	{
		error(mMappings.at(inputId), "Already mapped!");
		return false;
	}

	setAssignedTo(mMappings.at(inputId), input);
	
	input.configured = true;
	mTargetConfig->mapInput(inputName[inputId], input);

	LOG(LogInfo) << "  Mapping [" << input.string() << "] -> " << inputName[inputId];

	return true;
}

void GuiInputConfig::clearAssignment(int inputId)
{
	mTargetConfig->unmapInput(inputName[inputId]);
}
