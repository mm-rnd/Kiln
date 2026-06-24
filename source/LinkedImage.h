#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class LinkedImage : public juce::Component
{
public:
    LinkedImage(const juce::Image& img, const juce::String& url)
        : image(img), targetUrl(url) {}

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        g.drawImageWithin(image, b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                          juce::RectanglePlacement::centred);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mouseWasClicked())
            juce::URL(targetUrl).launchInDefaultBrowser();
    }

    // Optional: change cursor to hand on hover
    juce::MouseCursor getMouseCursor() override
    {
        return juce::MouseCursor::PointingHandCursor;
    }

private:
    juce::Image image;
    juce::String targetUrl;
};