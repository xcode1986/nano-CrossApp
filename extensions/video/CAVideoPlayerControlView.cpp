#include "CAVideoPlayerControlView.h"
NS_CC_EXT_BEGIN


CAVideoPlayerControlView::CAVideoPlayerControlView()
: m_glView(NULL)
, m_playButton(NULL)
, m_backButton(NULL)
, m_playSlider(NULL)
, m_playTimeLabel(NULL)
, m_bShowBackButton(false)
, m_bChanged(false)
, m_bPlaying(false)
, m_pPlayerControlViewDelegate(NULL)
, m_szTitle(UTF8("\u672a\u547d\u540d"))
{
	CAScheduler::schedule(schedule_selector(CAVideoPlayerControlView::updatePlayUI), this, 0);
}

CAVideoPlayerControlView::~CAVideoPlayerControlView()
{
	CAScheduler::unschedule(schedule_selector(CAVideoPlayerControlView::updatePlayUI), this);
}

CAVideoPlayerControlView* CAVideoPlayerControlView::createWithFrame(const DRect& rect)
{
	CAVideoPlayerControlView* pCtrlView = new CAVideoPlayerControlView();
	if (pCtrlView && pCtrlView->initWithFrame(rect))
	{
		pCtrlView->buildCtrlViews();
		pCtrlView->autorelease();
		return pCtrlView;
    }
	CC_SAFE_DELETE(pCtrlView);
	return pCtrlView;
}

CAVideoPlayerControlView* CAVideoPlayerControlView::createWithCenter(const DRect& rect)
{
	CAVideoPlayerControlView* pCtrlView = new CAVideoPlayerControlView();
	if (pCtrlView && pCtrlView->initWithCenter(rect))
	{
		pCtrlView->buildCtrlViews();
		pCtrlView->autorelease();
		return pCtrlView;
	}
	CC_SAFE_DELETE(pCtrlView);
	return pCtrlView;
}

CAVideoPlayerControlView* CAVideoPlayerControlView::createWithLayout(const DLayout& layout)
{
    CAVideoPlayerControlView* pCtrlView = new CAVideoPlayerControlView();
    if (pCtrlView && pCtrlView->initWithLayout(layout))
    {
        pCtrlView->autorelease();
        return pCtrlView;
    }
    CC_SAFE_DELETE(pCtrlView);
    return pCtrlView;
}

bool CAVideoPlayerControlView::init()
{
	if (!CAView::init())
	{
		return false;
	}

	this->setColor(ccc4(0, 0, 0, 255));
    this->buildCtrlViews();
	return true;
}

void CAVideoPlayerControlView::setShowBackButton(bool val)
{
	m_bShowBackButton = val;
	if (m_backButton)
	{
		m_backButton->setVisible(val);
	}
}

bool CAVideoPlayerControlView::getShowBackButton()
{
	return m_bShowBackButton;
}


void CAVideoPlayerControlView::setFullPath(const std::string& szPath)
{
	if (m_glView)
	{
		m_glView->setFullPath(szPath);
	}
}

void CAVideoPlayerControlView::setUrl(const std::string& szUrl)
{
	if (m_glView)
	{
		m_glView->setUrl(szUrl);
	}
}

void CAVideoPlayerControlView::setPlayerViewDelegate(CAVideoPlayerViewDelegate* var)
{
	if (m_glView)
	{
		m_glView->setPlayerViewDelegate(var);
	}
}


void CAVideoPlayerControlView::buildCtrlViews()
{
    m_glView = CAVideoPlayerView::createWithLayout(DLayoutFill);
    this->addSubview(m_glView);
    const std::map<std::string, std::string>& map = CAApplication::getApplication()->getThemeManager()->getThemeMap("CAVideoPlayer");
    // Bottom Panel Back
    CAImageView* bottomPanel = CAImageView::createWithLayout(DLayout(DHorizontalLayoutFill, DVerticalLayout_B_H(0, 188)));
    bottomPanel->setImage(CAImage::create(map.at("bottomPanel")));
    m_glView->addSubview(bottomPanel);
    
    // Slider
    CAImage* barImage = CAImage::create(map.at("barImage"));
    m_playSlider = CASlider::createWithLayout(DLayout(DHorizontalLayout_L_R(32, 32), DVerticalLayout_T_H(25, 56)));
    m_playSlider->setThumbTintImage(barImage);
    m_playSlider->addTarget(this, CAControl_selector(CAVideoPlayerControlView::onSlideDragging));
    m_playSlider->addTargetForTouchUpSide(this, CAControl_selector(CAVideoPlayerControlView::onSlideChanged));
    bottomPanel->addSubview(m_playSlider);
    
    // Play Pause Button
    CAImage* backImage = CAImage::create(map.at("playImage"));
    CAImage* backImage_h = CAImage::create(map.at("playImage_h"));
    m_playButton = CAButton::createWithLayout(DLayout(DHorizontalLayout_L_W(32, 56), DVerticalLayout_T_H(96, 56)), CAButtonTypeCustom);
    m_playButton->setImageForState(CAControlStateNormal, backImage);
    m_playButton->setImageForState(CAControlStateHighlighted, backImage_h);
    m_playButton->addTarget(this, CAControl_selector(CAVideoPlayerControlView::onButtonPause), CAControlEventTouchUpInSide);
    bottomPanel->addSubview(m_playButton);
    
    // play time
    m_playTimeLabel = CALabel::createWithLayout(DLayout(DHorizontalLayout_L_W(120, 200), DVerticalLayout_T_H(96, 56)));
    m_playTimeLabel->setFontSize(32);
    m_playTimeLabel->setVerticalTextAlignmet(CAVerticalTextAlignmentCenter);
    m_playTimeLabel->setColor(ccc4(255, 255, 255, 255));
    m_playTimeLabel->setText("00:00 / 00:00");
    bottomPanel->addSubview(m_playTimeLabel);

	updatePlayButton();
}

void CAVideoPlayerControlView::updatePlayButton()
{
    CC_RETURN_IF(m_bChanged == false);
    m_bChanged = false;
    const std::map<std::string, std::string>& map = CAApplication::getApplication()->getThemeManager()->getThemeMap("CAVideoPlayer");
	if (m_glView && m_glView->isPlaying()) 
	{
		CAImage* backImage = CAImage::create(map.at("pauseImage"));
		CAImage* backImage_h = CAImage::create(map.at("pauseImage_h"));
		m_playButton->setImageForState(CAControlStateNormal, backImage);
		m_playButton->setImageForState(CAControlStateHighlighted, backImage_h);
	} 
	else 
	{
		CAImage* backImage = CAImage::create(map.at("playImage"));
		CAImage* backImage_h = CAImage::create(map.at("playImage_h"));
		m_playButton->setImageForState(CAControlStateNormal, backImage);
		m_playButton->setImageForState(CAControlStateHighlighted, backImage_h);
	}
}

std::string CAVideoPlayerControlView::formatTimeInterval(float seconds, bool isLeft)
{
	seconds = MAX(0, seconds);

	int s = seconds;
	int m = s / 60;
	int h = m / 60;

	s = s % 60;
	m = m % 60;

	std::string format = (isLeft && seconds >= 0.5) ? "-" : "";
	char output[256] = { 0 };
	if (h != 0) {
		format.append("%d:%02d:%02d");
		sprintf(output, format.c_str(), h, m, s);
	}
	else {
		format.append("%02d:%02d");
		sprintf(output, format.c_str(), m, s);
	}

	return std::string(output);
}

void CAVideoPlayerControlView::onSlideDragging(CAControl* control, DPoint point)
{
    m_playTimeLabel->setText(formatTimeInterval(m_glView->getDuration() * m_playSlider->getValue(), false).append(" / ").append(formatTimeInterval(m_glView->getDuration() - 1, false)));
}

void CAVideoPlayerControlView::onSlideChanged(CAControl* control, DPoint point)
{
	float moviePosition = m_playSlider->getValue() * m_glView->getDuration();
	m_glView->setPosition(moviePosition);
	
	if (m_glView)
	{
		m_glView->play();
	}
}

void CAVideoPlayerControlView::onButtonPause(CAControl* control, DPoint point)
{
	if (m_glView == NULL)
		return;

    m_bChanged = true;
    
	if (m_glView->isPlaying())
	{
		m_glView->pause();
        m_bPlaying = false;
	}
	else
	{
		m_glView->play();
        m_bPlaying = true;
	}
	updatePlayButton();
}

void CAVideoPlayerControlView::onButtonBack(CAControl* control, DPoint point)
{
	if (m_pPlayerControlViewDelegate)
	{
		m_pPlayerControlViewDelegate->onBackButtonClicked(this);
	}
	this->removeFromSuperview();
}

void CAVideoPlayerControlView::updatePlayUI(float t)
{
	if (m_glView == NULL || m_playSlider == NULL || m_playTimeLabel == NULL)
		return;

	if (m_playSlider->isTouchClick())
		return;

	if (m_glView->isWaitSetPos())
		return;
	
    if (m_glView && m_glView->isPlaying())
    {
        if (m_bPlaying == false)
        {
            m_bChanged = true;
        }
        m_bPlaying = true;
    }
    else
    {
        if (m_bPlaying == true)
        {
            m_bChanged = true;
        }
        m_bPlaying = false;
    }
    
	updatePlayButton();

	const float duration = m_glView->getDuration();
	const float position = m_glView->getPosition();

	m_playSlider->setValue(position / duration);
	m_playTimeLabel->setText(formatTimeInterval(position, false).append(" / ").append(formatTimeInterval(duration - 1, false)));
}

NS_CC_EXT_END
