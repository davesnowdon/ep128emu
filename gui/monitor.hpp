
// ep128emu -- portable Enterprise 128 emulator
// Copyright (C) 2003-2009 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/ep128emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef EP128EMU_GUI_MONITOR_HPP
#define EP128EMU_GUI_MONITOR_HPP

#include "gui.hpp"

#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>

class Ep128EmuGUIMonitor : public Fl_Text_Editor {
 protected:
  Fl_Text_Buffer            *buf_;
  Ep128EmuGUI_DebugWindow   *debugWindow;
  Ep128EmuGUI               *gui;
  int32_t                   assembleOffset;
  uint32_t                  disassembleAddress;
  int32_t                   disassembleOffset;
  uint32_t                  memoryDumpAddress;
  uint32_t                  addressMask;
  bool                      cpuAddressMode;
  uint8_t                   traceFlags;
  std::FILE                 *traceFile;
  size_t                    traceInstructionsRemaining;
  // --------
  void command_assemble(const std::vector<std::string>& args);
  void command_disassemble(const std::vector<std::string>& args);
  void command_memoryDump(const std::vector<std::string>& args);
  void command_memoryModify(const std::vector<std::string>& args);
  void command_ioDump(const std::vector<std::string>& args);
  void command_ioModify(const std::vector<std::string>& args);
  void command_printRegisters(const std::vector<std::string>& args);
  void command_setRegisters(const std::vector<std::string>& args);
  void command_go(const std::vector<std::string>& args);
  void command_searchPattern(const std::vector<std::string>& args);
  void command_searchAndReplace(const std::vector<std::string>& args);
  void command_memoryCopy(const std::vector<std::string>& args);
  void command_memoryFill(const std::vector<std::string>& args);
  void command_memoryCompare(const std::vector<std::string>& args);
  void command_assemblerOffset(const std::vector<std::string>& args);
  void command_printInfo(const std::vector<std::string>& args);
  void command_continue(const std::vector<std::string>& args);
  void command_step(const std::vector<std::string>& args);
  void command_stepOver(const std::vector<std::string>& args);
  void command_trace(const std::vector<std::string>& args);
  void command_load(const std::vector<std::string>& args,
                    bool verifyMode = false);
  void command_save(const std::vector<std::string>& args);
  void command_toggleCPUAddressMode(const std::vector<std::string>& args);
  void command_help(const std::vector<std::string>& args);
  static int enterKeyCallback(int c, Fl_Text_Editor *e_);
  void moveDown();
  void parseCommand(const char *s);
  void disassembleInstruction(bool assembleMode = false);
  void memoryDump();
  void printCPURegisters1();
  void printCPURegisters2();
  void parseSearchPattern(std::vector<uint8_t>& searchString_,
                          std::vector<uint8_t>& searchMask_,
                          const std::vector<std::string>& args,
                          size_t argOffs, size_t argCnt);
  int32_t searchPattern(const std::vector<uint8_t>& searchString_,
                        const std::vector<uint8_t>& searchMask_,
                        uint32_t startAddr, uint32_t endAddr,
                        bool cpuAddressMode_);
 public:
  Ep128EmuGUIMonitor(int xx, int yy, int ww, int hh, const char *ll = 0);
  virtual ~Ep128EmuGUIMonitor();
  void setDebugger(Ep128EmuGUI_DebugWindow& debugWindow_)
  {
    debugWindow = &debugWindow_;
    gui = &(debugWindow->gui);
  }
  void printMessage(const char *s);
  void breakMessage(const char *s = 0);
  int32_t searchPattern(const std::vector<std::string>& args,
                        size_t argOffs, size_t argCnt,
                        uint32_t startAddr, uint32_t endAddr,
                        bool cpuAddressMode_);
  inline bool getIsTraceOn() const
  {
    return (traceFile != (std::FILE *) 0);
  }
  void writeTraceFile(uint16_t addr);
  void closeTraceFile();
};

#endif  // EP128EMU_GUI_MONITOR_HPP

