// The MIT License (MIT)
//
// Copyright (c) 2018 Jan Kuri <jkuri88@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// Copyright 2021 Ellogon BV.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef __SPINNERS_H_
#define __SPINNERS_H_
#include <cstring>
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>

namespace spinners {
  inline std::map<const char *, const char *> SpinnerType = {
    {"dots", u8"⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"}, {"dots2", u8"⣾⣽⣻⢿⡿⣟⣯⣷"}, {"dots3", u8"⠋⠙⠚⠞⠖⠦⠴⠲⠳⠓"},
    {"dots4", u8"⠄⠆⠇⠋⠙⠸⠰⠠⠰⠸⠙⠋⠇⠆"}, {"dots5", u8"⠋⠙⠚⠒⠂⠂⠒⠲⠴⠦⠖⠒⠐⠐⠒⠓⠋"},
    {"dots6", u8"⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠴⠲⠒⠂⠂⠒⠚⠙⠉⠁"}, {"dots7", u8"⠈⠉⠋⠓⠒⠐⠐⠒⠖⠦⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈"},
    {"dots8", u8"⠁⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈⠈"}, {"dots9", u8"⢹⢺⢼⣸⣇⡧⡗⡏"},
    {"dots10", u8"⢄⢂⢁⡁⡈⡐⡠"}, {"dots11", u8"⠁⠂⠄⡀⢀⠠⠐⠈"}, {"pipe", u8"┤┘┴└├┌┬┐"},
    {"star", u8"✶✸✹✺✹✷"}, {"star2", u8"+x*"}, {"flip", u8"___-``'´-___"},
    {"hamburger", u8"☱☲☴"}, {"growVertical", u8"▁▃▄▅▆▇▆▅▄▃"},
    {"growHorizontal", u8"▏▎▍▌▋▊▉▊▋▌▍▎"}, {"balloon", u8" .oO@* "},
    {"balloon2", u8".oO°Oo."}, {"noise", u8"▓▒░"}, {"bounce", u8"⠁⠂⠄⠂"},
    {"boxBounce", u8"▖▘▝▗"}, {"boxBounce2", u8"▌▀▐▄"}, {"triangle", u8"◢◣◤◥"},
    {"arc", u8"◜◠◝◞◡◟"}, {"circle", u8"◡⊙◠"}, {"squareCorners", u8"◰◳◲◱"},
    {"circleQuarters", u8"◴◷◶◵"}, {"circleHalves", u8"◐◓◑◒"}, {"squish", u8"╫╪"},
    {"toggle", u8"⊶⊷"}, {"toggle2", u8"▫▪"}, {"toggle3", u8"□■"}, {"toggle4", u8"■□▪▫"},
    {"toggle5", u8"▮▯"}, {"toggle6", u8"ဝ၀"}, {"toggle7", u8"⦾⦿"}, {"toggle8", u8"◍◌"},
    {"toggle9", u8"◉◎"}, {"toggle10", u8"㊂㊀㊁"}, {"toggle11", u8"⧇⧆"},
    {"toggle12", u8"☗☖"}, {"toggle13", u8"=*-"}, {"arrow", u8"←↖↑↗→↘↓↙"}};

  inline const char *GetSpinner(const char *key) {
    auto search = SpinnerType.find(key);
    if (search != SpinnerType.end()) {
      return search->second;
    } else {
      search = SpinnerType.find("dots");
      return search->second;
    }
  }

  class Spinner {
  public:
    Spinner() : interval_(80), text_(""), stop_spinner_(false), symbols_(GetSpinner("dots")) {}
    Spinner(int interval, std::string text, const char *symbols)
      : interval_(interval), text_(text), stop_spinner_(false), symbols_(GetSpinner(symbols)) {}
    ~Spinner() { Stop(); };

    void SetInterval(int interval) { interval_ = interval; }
    void SetText(std::string text) { text_ = text; }
    void SetSymbols(const char *symbols) { symbols_ = GetSpinner(symbols); }

    void StartSpinner() {
      int len = strlen(symbols_) / 3;
      int i = 0;
      char ch[4] = {};

      HideCursor();
      while (!stop_spinner_) {
        i = (i >= (len - 1)) ? 0 : i + 1;
        std::cout << "\u001b[K";
        strncpy(ch, symbols_ + i * 3, 3);
        std::cout << ch << " " << text_ << " \r";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
      }
      ShowCursor();
    }

    void Start() {
      t_ = std::thread(&Spinner::StartSpinner, this);
    }

    void Stop() {
      stop_spinner_ = true;
      if (t_.joinable()) {
        t_.join();
      }
    }

  private:
    int interval_;
    std::string text_;
    bool stop_spinner_;
    const char *symbols_;
    std::thread t_;

    void HideCursor() {
      std::cout << "\u001b[?25l";
    }

    void ShowCursor() {
      std::cout << "\u001b[?25h";
      std::cout.flush();
    }
  };
}
#endif // __SPINNERS_H_
