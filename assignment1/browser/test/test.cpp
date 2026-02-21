#include <gtest/gtest.h>
#include <browser.hpp>



// visit google.com
// back 2
// back 1
// forward 1

TEST(BrowserSimpleTests, SimpleCases)
{
    Browser browser;

    browser.visit("google");

    browser.back(2);
    browser.back(1);
    browser.forward(1);

    String s = browser.dumpHistory();

    EXPECT_STREQ(s.getConstData(), "homepage|google|homepage|homepage|google");
}

TEST(BrowserSimpleTests, SimpleCases2)
{
    Browser browser;

    browser
    .visit("duplo")
    .visit("suplo")
    .back(2)
    .forward(2)
    .forward(1)
    .visit("fuplo");

    String s = browser.dumpHistory();

    EXPECT_STREQ(s.getConstData(), "homepage|duplo|suplo|homepage|suplo|suplo|fuplo");
}