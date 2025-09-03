
///
/// Original repo: https://github.com/Casqade/BF1942_patcher
///
///
/// Building:
///
/// cl /std:c++17 /EHsc BF1942_patcher.cpp && rm BF1942_patcher.obj
///
/// g++ --std=c++17 -static BF1942_patcher.cpp -o BF1942_patcher.exe
///


#include <cstdint>
#include <limits>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>


enum ExecutableType : bool
{
  Main,
  Debug,

} exeType {};

const std::string executableName[]
{
  "BF1942.exe",
  "BF1942_r.exe",
};

const std::string blackScreenExeName[]
{
  "BlackScreen.exe",
  "BlackScreen_r.exe",
};

const size_t executableSize[]
{
  5'648'384,
  8'454'144,
};

template <typename T>
void
PromptNumber(
  T& number,
  T rangeMin,
  T rangeMax,
  const std::string& numberName )
{
  while ( true )
  {
    std::cout << "Set " << numberName
              << " or leave empty to use default ("
              << static_cast <int> (number) << "): ";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if ( userInput.empty() == true )
      break;

    try
    {
      int userNumber = std::stoll(userInput);

      if ( userNumber >= rangeMin && userNumber <= rangeMax )
      {
        number = userNumber;
        break;
      }
    }
    catch ( ... )
    {
    }

    std::cout << "ERROR: Unsupported " << numberName << " '" << userInput << "'\n\n";
  }

  std::cout << "\n";
};


using TextValidationCallback = bool( const std::string& );

void
PromptText(
  std::string& text,
  TextValidationCallback* validateText,
  const std::string& textName )
{
  while ( true )
  {
    std::cout
      << "Set " << textName
      << " or leave empty to use default '"
      << text << "': ";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if ( userInput.empty() == true )
      break;


    if ( validateText(userInput) == true )
    {
      text = userInput;
      break;
    }

    std::cout << "\n";
  }

  std::cout << "\n";
}


namespace Patches
{

enum class OperationMode
{
  Skip,
  Modify,
  Restore,
};

template <typename T>
std::vector <uint8_t>
ToBytes(
  T&& type )
{
  std::vector <uint8_t> bytes(sizeof(T));

  std::memcpy(
    bytes.data(),
    std::addressof(type), sizeof(T) );

  return bytes;
}

template <>
std::vector <uint8_t>
ToBytes(
  const std::string& text )
{
  std::vector <uint8_t> bytes(text.size() + 1);

  std::memcpy(
    bytes.data(),
    text.data(),
    text.size() );

  return bytes;
}

template <>
std::vector <uint8_t>
ToBytes(
  std::string&& text )
{
  return ToBytes(
    std::forward <const std::string&> (text) );

  const auto& str = text;
//  const std::string str {std::move(text)};
//  const std::string& str {std::move(text)};

  return ToBytes(str);
}

void
ApplyPatch(
  std::fstream& file,
  std::ios::off_type offset,
  const std::vector <uint8_t> bytes )
{
  file.seekp( offset, std::ios::beg );

  file.write(
    reinterpret_cast <const char*> (bytes.data()),
    bytes.size() );
}

void
ApplyPatch(
  std::fstream& file,
  ExecutableType exeType,
  std::ios::off_type offsetMain,
  std::ios::off_type offsetDebug,
  const std::vector <uint8_t> bytes )
{
  const std::ios::off_type offset[]
  { offsetMain, offsetDebug };

  file.seekp( offset[exeType], std::ios::beg );

  file.write(
    reinterpret_cast <const char*> (bytes.data()),
    bytes.size() );
}

void
ApplyPatch(
  std::fstream& file,
  OperationMode mode,
  std::ios::off_type offset,
  const std::vector <uint8_t>& originalBytes,
  const std::vector <uint8_t>& modifiedBytes )
{
  if ( mode == OperationMode::Modify )
    ApplyPatch(file, offset, modifiedBytes);

  else if ( mode == OperationMode::Restore )
    ApplyPatch(file, offset, originalBytes);
}

void
ApplyPatch(
  std::fstream& file,
  OperationMode mode,
  ExecutableType exeType,
  std::ios::off_type offsetMain,
  std::ios::off_type offsetDebug,
  const std::vector <uint8_t>& originalBytes,
  const std::vector <uint8_t>& modifiedBytes )
{
  const std::ios::off_type offset[]
  { offsetMain, offsetDebug };

  ApplyPatch(
    file, mode,
    offset[exeType],
    originalBytes,
    modifiedBytes );
}

void
ApplyPatch(
  std::fstream& file,
  OperationMode mode,
  ExecutableType exeType,
  std::ios::off_type offsetMain,
  const std::vector <uint8_t>& originalBytesMain,
  const std::vector <uint8_t>& modifiedBytesMain,
  std::ios::off_type offsetDebug,
  const std::vector <uint8_t>& originalBytesDebug,
  const std::vector <uint8_t>& modifiedBytesDebug )
{
  if ( exeType == ExecutableType::Main )
    ApplyPatch(
      file, mode,
      offsetMain,
      originalBytesMain,
      modifiedBytesMain );

  else if ( exeType == ExecutableType::Debug )
    ApplyPatch(
      file, mode,
      offsetDebug,
      originalBytesDebug,
      modifiedBytesDebug );
}


class Patch
{
protected:
  OperationMode mOperationMode {};


public:
  Patch() = default;
  virtual ~Patch() {};

  virtual void promptUser( ExecutableType )
  {}

  virtual void patch( std::fstream& executable, ExecutableType exeType ) const
  {}


protected:

  virtual const char* description() const
  { return "Not implemented"; }


  static void ReadBytes(
    std::fstream& file, int offset,
    char* dst, size_t count )
  {
    file.seekp( offset, std::ios::beg );
    file.read( dst, count );
  }


  void PromptOperationMode()
  {
    while ( true )
    {
      std::cout << description() << "\n"
        "  Enable? (1/2/3):\n"
        "    1. skip (default, don't write to executable)\n"
        "    2. no (restore bytes from original binary)\n"
        "    3. yes\n";

      std::string userInput {};
      std::getline(std::cin, userInput);

      if (  userInput.empty() == true ||
            userInput == "1" ||
            userInput == "s" ||
            userInput == "skip" )
      {
        mOperationMode = OperationMode::Skip;
        break;
      }

      if (  userInput == "2" ||
            userInput == "n" ||
            userInput == "no" )
      {
        mOperationMode = OperationMode::Restore;
        break;
      }

      if (  userInput == "3" ||
            userInput == "y" ||
            userInput == "yes" )
      {
        mOperationMode = OperationMode::Modify;
        break;
      }

      std::cout << "ERROR: Unrecognized input: '" << userInput << "'\n\n";
    }

    std::cout << "\n";
  }
};


class Handle4GBAddressSpace : public Patch
{
  const char* description() const override
  {
    return
      "-----------------------------\n"
      "--- Handle4GBAddressSpace ---\n"
      "-----------------------------\n"
      "This tweak allows the game to access more memory (4 GB).";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
    const std::ios::off_type flagsOffset = 0x14e;
    int16_t characteristicsFlags {};

    ReadBytes(
      file, flagsOffset,
      reinterpret_cast <char*> (&characteristicsFlags),
      sizeof(characteristicsFlags) );

    updateCharacteristicsFlags(characteristicsFlags);

    ApplyPatch(
      file, flagsOffset,
      ToBytes(characteristicsFlags) );
  }


private:

  void updateCharacteristicsFlags( int16_t& flags ) const
  {
//    IMAGE_FILE_LARGE_ADDRESS_AWARE
    const int16_t largeAddressAwareFlag {0x0020};

    switch (mOperationMode)
    {
      case OperationMode::Skip:
        return;

      case OperationMode::Modify:
        flags |= largeAddressAwareFlag;
        return;

      case OperationMode::Restore:
        flags &= ~largeAddressAwareFlag;
        return;
    }
  }
};


class DefaultWindowPosition : public Patch
{
  const int8_t DefaultWindowPosX {50};
  const int8_t DefaultWindowPosY {50};

  int8_t mWindowPosX {DefaultWindowPosX};
  int8_t mWindowPosY {DefaultWindowPosY};


  const char* description() const override
  {
    return
      "-----------------------------\n"
      "--- DefaultWindowPosition ---\n"
      "-----------------------------\n"
      "When the game is launched in windowed mode,\nthe window "
      "is positioned at screen offset [50, 50].\n"
      "This tweak allows to change this offset.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;


    auto offsetMin = std::numeric_limits <decltype(mWindowPosX)>::min();
    auto offsetMax = std::numeric_limits <decltype(mWindowPosX)>::max();

    std::cout
      << "Valid window offset range: ["
      << std::to_string(offsetMin) << ", " << std::to_string(offsetMax) << "]\n\n";

    PromptNumber(
      mWindowPosX,
      offsetMin, offsetMax,
      "window X offset" );

    PromptNumber(
      mWindowPosY,
      offsetMin, offsetMax,
      "window Y offset" );
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2325bd, 0x2e1f43,
      ToBytes(DefaultWindowPosX),
      ToBytes(mWindowPosX) );

    ApplyPatch(
      file, mOperationMode, exeType,
      0x2325bb, 0x2e1f41,
      ToBytes(DefaultWindowPosY),
      ToBytes(mWindowPosY) );
  }
};


class ForceMaximizedWindow : public Patch
{
public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();
  }

  const char* description() const override
  {
    return
      "----------------------------\n"
      "--- ForceMaximizedWindow ---\n"
      "----------------------------\n"
      "This tweak makes game window maximized by default.";
  }


protected:

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    patch dwStyle for CreateWindow()
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2325c2, 0x2e1f48,
      { 0x00 },   // ~WS_MAXIMIZE
      { 0x01 } ); // WS_MAXIMIZE

//    in case CreateWindow() ignores WS_MAXIMIZE style,
//    let's patch nCmdShow flags for ShowWindow() as well
    ApplyPatch(
      file, mOperationMode, exeType,
      0x23260b, 0x2e1fe3,
      { 0x05 },   // SW_SHOW
      { 0x03 } ); // SW_SHOWMAXIMIZED
  }
};


class DefaultWindowResolution : public Patch
{
  const int DefaultWidth {800};
  const int DefaultHeight {600};

  const int MinWidth {640};
  const int MinHeight {480};
  const int MaxResolution {32768};

  int mWidth {DefaultWidth};
  int mHeight {DefaultHeight};


  const char* description() const override
  {
    return
      "-------------------------------\n"
      "--- DefaultWindowResolution ---\n"
      "-------------------------------\n"
      "This tweak sets default game window resolution to a custom value.\n"
      "Technically, it should prevent the game from messing up your\n"
      "monitor/desktop settings when launching in fullscreen mode";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;

    std::cout <<
      "Valid resolution range: 640 x 480 up to 32768 x 32768\n"
      "(upper one is OS limit, don't expect the game to tolerate that)\n\n";

    PromptNumber(
      mWidth, MinWidth, MaxResolution,
      "window width" );

    PromptNumber(
      mHeight, MinHeight, MaxResolution,
      "window height" );
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
    struct ResolutionOffsets
    {
      int width {};
      int height {};
    };

    ResolutionOffsets offsets[][2]
    {

//      RenderSetup
//      Fallback resolution when either video mode or profile is not found.
//      Search process:
//        1. Read game.setProfile value from Settings/Profile.con
//        2. Open Settings/Profiles/[profilename]/Video.con
//        3. Find game.setDisplayMode & use its value
//      Note: the game searches for game.setDisplayMode instead of game.setGameDisplayMode,
//            therefore it always fails and fallbacks. Unfortunately, there isn't enough
//            space in main executable to patch it without using DLL injection (e.g. bf42plus)
      {
        { 0x631ef, 0x631f7 },
        { 0xa34e7, 0xa34f1 },
      },

//      NullDRendererInit: create viewport
      {
        { 0x240940, 0x240947 },
        { 0x2f7018, 0x2f7022 },
      },

//      Options/Video/ChangeVideoPerformance
//      Settings/Default/VideoCustom.con
      {
        { 0x2af66b, 0x2af673 },
        { 0x385cbf, 0x385cc6 },
      },

//      MainMenu::MainMenu()
//      VideoOptions::VideoOptions()
      {
        { 0x2b0281, 0x2b0288 },
        { 0x386711, 0x3866d1 },
      },
      {
        { 0x2b0299, 0x2b02a0 },
        { 0x38671e, 0x3866d1 },
                    // both this & previous entry height
                    // values are moved from the same register
      },

//      BFMenu::switch
//      Set default video mode
      {
        { 0x2b1143, 0x2b114a },
        { 0x387845, 0x38784c },
      },

//      VideoOptions::SaveVideoOptions
//      Settings/Profiles/Custom/Video.con
      {
        { 0x2b13f7, 0x2b13ff },
        { 0x387c07, 0x387c0e },
      },

//      VideoOptions::SaveVideoOptions
//      Settings/Profiles/Default/VideoCustom.con
      {
        { 0x2b184b, 0x2b1853 },
        { 0x388046, 0x38804d },
      },
    };

    const auto defaultWidth = ToBytes(DefaultWidth);
    const auto defaultHeight = ToBytes(DefaultHeight);

    const auto width = ToBytes(mWidth);
    const auto height = ToBytes(mHeight);


    for ( auto&& offset : offsets )
    {
      ApplyPatch(
        file, mOperationMode,
        offset[exeType].width,
        defaultWidth, width );

      ApplyPatch(
        file, mOperationMode,
        offset[exeType].height,
        defaultHeight, height );
    }
  }
};


class DontSetDefaultVideoMode : public Patch
{
  const char* description() const override
  {
    return
      "-------------------------------\n"
      "--- DontSetDefaultVideoMode ---\n"
      "-------------------------------\n"
      "This tweak prevents setting default video mode\n"
      "in favour of loading it from user profile config files.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
    ApplyPatch(
      file, mOperationMode, exeType,
      0x5DD6A,
        { 0xD2, 0x33 },
        { 0x02, 0x34 },
      0x97E43,
        { 0xF7, 0xF9 },
        { 0x15, 0xFA } );
  }
};


class FixAspectRatio : public Patch
{
  const char* description() const override
  {
    return
      "----------------------\n"
      "--- FixAspectRatio ---\n"
      "----------------------\n"
      "This tweak fixes aspect ratio for widescreens.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    dice.ref2.rend.RenderView.Standard member
//    Camera aspect ratio?
    ApplyPatch(
      file, mOperationMode, exeType,
      0x1B7D22, 0x23DAE0,
      { 0x7B },
      { 0xEB } );

//    Options/Video/LoadVideoOptions
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2B0909,
      { 0x0F, 0x8A, 0xB9, 0x00, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      0x386CBF,
      { 0x0F, 0x8A, 0xF0, 0x01, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } );
  }
};


class InhibitAddToBuddyListFailedMessage : public Patch
{
  const char* description() const override
  {
    return
      "------------------------------------------\n"
      "--- InhibitAddToBuddyListFailedMessage ---\n"
      "------------------------------------------\n"
      "This tweak removes console output for error\n"
      "'failed to add player to buddylist. Couldn't locate player'.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    game.addPlayerToBuddyListByName
    ApplyPatch(
      file, mOperationMode, exeType,
      0x6A410,
        { 0xE8, 0xEB, 0xD8, 0x23, 0x00 },
        { 0x83, 0xC4, 0x1C, 0x90, 0x90 },
      0xAD7EB,
        { 0xE8, 0xF5, 0x10, 0x2D, 0x00 },
        { 0x83, 0xC4, 0x1C, 0x90, 0x90 } );
  }
};


class CustomMasterServer : public Patch
{
  inline const static std::string DefaultMasterServer {"master.gamespy.com"};

  std::string mMasterServer {DefaultMasterServer}; // master.bf1942.org


  const char* description() const override
  {
    return
      "--------------------------\n"
      "--- CustomMasterServer ---\n"
      "--------------------------\n"
      "This tweak sets custom master server.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;

    PromptText(
      mMasterServer,
      CheckMasterServer,
      "master server");
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    clear with zeroes first
    if ( mOperationMode != OperationMode::Skip )
    {
      ApplyPatch( file, exeType,
        0x557C30, 0x7FC3D8,
        std::vector <uint8_t> (DefaultMasterServer.size()) );

      ApplyPatch( file, exeType,
        0x557DF8, 0x7FC568,
        std::vector <uint8_t> (DefaultMasterServer.size()) );
    }

    ApplyPatch(
      file, mOperationMode, exeType,
      0x557C30, 0x7FC3D8,
      ToBytes(DefaultMasterServer),
      ToBytes(mMasterServer) );

    ApplyPatch(
      file, mOperationMode, exeType,
      0x557DF8, 0x7FC568,
      ToBytes(DefaultMasterServer),
      ToBytes(mMasterServer) );
  }


private:

  static bool CheckMasterServer( const std::string& masterServer )
  {
    if ( masterServer.empty() == true )
    {
      std::cout << "ERROR: Master server can't be empty\n";
      return false;
    }

//    Technically, there's enough space to fit up to 63 characters,
//    but it's safer to avoid exceeding default master server length
    if ( masterServer.size() > DefaultMasterServer.size() )
    {
      std::cout << "ERROR: Master server can't be longer than "
                << DefaultMasterServer.size() << " characters\n";

      return false;
    }

    for ( auto& ch : masterServer )
    {
//      Requirements for fully qualified domain name
//      https://datatracker.ietf.org/doc/html/rfc952

      if ( ch >= 'A' && ch <= 'Z' )
        continue;

      if ( ch >= 'a' && ch <= 'z' )
        continue;

      if ( ch >= '0' && ch <= '9' )
        continue;

      if ( ch == '-' || ch == '.' )
        continue;

      std::cout <<
        "ERROR: Invalid character '" << ch << "'. Master server may only contain"
        "       letters (a-z, A-Z), digits (0-9), minus signs (-), and periods (.)\n";
      return false;
    }

    if ( masterServer.back() == '.' )
    {
      std::cout << "ERROR: Master server isn't allowed to end with a period (.)\n";
      return false;
    }

    if ( masterServer.back() == '-' )
    {
      std::cout << "ERROR: Master server isn't allowed to end with a minus sign (-)\n";
      return false;
    }

    return true;
  }
};


class CustomWindowTitle : public Patch
{
  inline const static std::string DefaultWindowTitle[2]
  {
    "BF1942 (Ver: Tue, 19 Oct 2004 14:58:45)",
    "BF1942 (Ver: Thu, 29 Jan 2004 11:34:56)",
  };

  std::string mWindowTitle
  {
    DefaultWindowTitle[ExecutableType::Main]
  };


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;

    PromptText(
      mWindowTitle,
      CheckWindowTitle,
      "window title");
  }

  const char* description() const override
  {
    return
      "-------------------------\n"
      "--- CustomWindowTitle ---\n"
      "-------------------------\n"
      "This tweak sets custom game window title.";
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    clear with zeroes first
    if ( mOperationMode != OperationMode::Skip )
      ApplyPatch( file, exeType,
        0x4D417C, 0x709C50,
        std::vector <uint8_t> (MaxWindowTitleLength()) );

    ApplyPatch(
      file, mOperationMode, exeType,
      0x4D417C, 0x709C50,
      ToBytes(DefaultWindowTitle[exeType]),
      ToBytes(mWindowTitle) );
  }


private:

  static bool CheckWindowTitle( const std::string& windowTitle )
  {
    if ( windowTitle.size() >= MaxWindowTitleLength() )
    {
      std::cout
        << "ERROR: Window title can't be longer than "
        << MaxWindowTitleLength() << " characters\n";

      return false;
    }

    return true;
  }

  static size_t MaxWindowTitleLength()
  {
    return std::max(
      DefaultWindowTitle[0].size(),
      DefaultWindowTitle[1].size() );
  }
};


class MaxBotCount : public Patch
{
  const uint8_t DefaultMaxBotCount {64};
  uint8_t mMaxBotCount {DefaultMaxBotCount};


  const char* description() const override
  {
    return
      "-------------------\n"
      "--- MaxBotCount ---\n"
      "-------------------\n"
      "This tweak increases maximum bot count.";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;


    const auto maxBotCountLimit =
      std::numeric_limits <decltype(DefaultMaxBotCount)>::max();

    std::cout
      << "Valid bot count range: ["
      << std::to_string(DefaultMaxBotCount) << ", "
      << std::to_string(maxBotCountLimit) << "]\n\n";

    PromptNumber(
      mMaxBotCount,
      DefaultMaxBotCount,
      maxBotCountLimit,
      "max bot count" );
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    game.serverMaxPlayers
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2195F, 0x455E3,
      { DefaultMaxBotCount },
      { mMaxBotCount } );

    ApplyPatch(
      file, mOperationMode, exeType,
      0x2196A, 0x455E7,
      { DefaultMaxBotCount },
      { mMaxBotCount } );

    ApplyPatch(
      file, mOperationMode, exeType,
      0x21967, 0x455E4,
      { 0x7E },
      { 0x76 } );
//

    if ( exeType == ExecutableType::Debug )
      return;

//    TODO: are these really necessary?

    ApplyPatch(
      file, mOperationMode,
      0x5857,
      { DefaultMaxBotCount },
      { mMaxBotCount } );

    ApplyPatch(
      file, mOperationMode,
      0x585F,
      { DefaultMaxBotCount },
      { mMaxBotCount } );

    ApplyPatch(
      file, mOperationMode,
      0x585C,
      { 0x7E },
      { 0x76 } );
  }
};


class InternetGamesSupport : public Patch
{
  const char* description() const override
  {
    return
      "----------------------------\n"
      "--- InternetGamesSupport ---\n"
      "----------------------------\n"
      "This tweak enables internet games support, allowing manual\n"
      "IP:port connection when server browser fails to find a LAN game.\n"
      "Note that BF1942_r.exe is incompatible with regular executables/servers,\n"
      "this patch simply allows to connect by IP without having to launch\n"
      "the game with cmd arguments like +joinServer and +isInternet";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    if ( exeType == ExecutableType::Debug )
      PromptOperationMode();
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
    if ( exeType == ExecutableType::Main )
      return;

//    HostGame
    ApplyPatch(
      file, mOperationMode,
      0x8AA48, // ~0x538CA in main executable
      { 0x38, 0x9E, 0x58, 0x05, 0x00, 0x00, 0x74, 0x6A },
      { 0xEB, 0x70, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } );

//    JoinServer
    ApplyPatch(
      file, mOperationMode,
      0x8C367, // ~0x54588 in main executable
      { 0x74 },
      { 0xEB });

//    AddJoinServer
    ApplyPatch(
      file, mOperationMode,
      0x377287, // ~0x2A057D in main executable
      { 0x83, 0x3D, 0x04, 0x61, 0xC0, 0x00, 0x00, 0x75, 0x34 },
      { 0xEB, 0x3B, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } );
  }
};


class MakePortable : public Patch
{
//  identical to hashsum at BF1942_w32ded.exe:2E4F3C
  const unsigned int DefaultExeCRC {0xA14AAC64};
  unsigned int mExeCrc {DefaultExeCRC};


  const char* description() const override
  {
    return
      "--------------------\n"
      "--- MakePortable ---\n"
      "--------------------\n"
      "This tweak makes the game portable, i.e.:\n"
      "  - no CD and/or CD drive is required\n"
      "  - no need for CD-key in the registry (you'll probably need it to play on public servers though)\n"
      "  - DLL checksum verification is removed (you'll be able to use SiMPLE's 4 KB Mod.dll)\n"
      "  - executable CRC check is replaced with a constant hashum so that\n"
      "    both ingame & SiMPLE's patched dedicated servers will accept any client executable)\n";
  }


public:

  void promptUser( ExecutableType exeType ) override
  {
    PromptOperationMode();

    if ( mOperationMode != OperationMode::Modify )
      return;

//    TODO: prompt for mExeCrc? Probably not really necessary
    return;

    PromptNumber(
      mExeCrc,
      std::numeric_limits <decltype(mExeCrc)>::min(),
      std::numeric_limits <decltype(mExeCrc)>::max(),
      "server-approved executable CRC" );
  }

  void patch( std::fstream& file, ExecutableType exeType ) const override
  {
//    ignore bf1942 CD-key check for local player
    ApplyPatch(
      file, mOperationMode, exeType,
      0xCD5E, 0x29CA7,
      { 0x75 },
      { 0xEB } );

//    skip optical drive enumeration
    ApplyPatch(
      file, mOperationMode, exeType,
      0x4D41D,
      { 0xE8, 0xEE, 0xFE, 0xFF, 0xFF },
      { 0xB8, 0x01, 0x00, 0x00, 0x00 },
      0x83CF8,
      { 0xE8, 0xD1, 0x7C, 0xF8, 0xFF },
      { 0xB8, 0x01, 0x00, 0x00, 0x00 } );

    if ( exeType == ExecutableType::Main )
    {
//      Host/Create/HostInternetGame
//      Host/Internet/HostLanGame
//      Skirmish/StartSkirmish
//      Join/Internet/EnterAction
//      Join/Internet/JoinInternetGame
//      Join/Lan/JoinLanGame
//      CustomGame/ActivateCustomGame
//        skip optical drive enumeration
//          for CD:\\setup.ini: Startup/ProductGUID check
      ApplyPatch(
        file, mOperationMode,
        0x4DDC9,
        { 0xE8, 0x42, 0xF5, 0xFF, 0xFF },
        { 0xB8, 0x01, 0x00, 0x00, 0x00 } );

//      Join/Internet/EnterAction
//      Join/Internet/JoinInternetGame
//      Join/Lan/JoinLanGame
//      CustomGame/ActivateCustomGame
//        skip optical drive enumeration
//          for CD:\\bfdist.vlu check
      ApplyPatch(
        file, mOperationMode,
        0x4DEFF,
        { 0xE8, 0x0C, 0xF4, 0xFF, 0xFF },
        { 0xB8, 0x01, 0x00, 0x00, 0x00 } );

//      ignore invalid DLL checksum
      ApplyPatch(
        file, mOperationMode,
        0x58364,
        { 0x0F, 0x85 },
        { 0x90, 0xE9 } );
    }

//    pretend RtR CD-key is present in the registry
    if ( exeType == ExecutableType::Debug )
//      main executable uses only 1 call for both DLCs
      ApplyPatch(
        file, mOperationMode,
        0x92B42,
        { 0x0F, 0x85 },
        { 0x90, 0xE9 } );

//    pretend SWoWWII CD-key is present in the registry
    ApplyPatch(
      file, mOperationMode, exeType,
      0x59EED, 0x92BFE,
      { 0x75 },
      { 0xEB } );

//    insert CD
    ApplyPatch(
      file, mOperationMode, exeType,
      0x5BDBE, 0x9558D,
      { 0x74 },
      { 0xEB } );

//    check connection to master server?
    ApplyPatch(
      file, mOperationMode, exeType,
      0x7F8E5,
      { 0xE8, 0x76, 0xFB, 0xFF, 0xFF },
      { 0xB8, 0x00, 0x00, 0x00, 0x00 },
      0xCE0B9,
      { 0xE8, 0x48, 0xFB, 0xFF, 0xFF },
      { 0xB8, 0x00, 0x00, 0x00, 0x00 } );


//    GameServer::StartServer
//      use server-approved value for executable CRC32
    ApplyPatch(
      file, mOperationMode, exeType,

      0x8765F,
      { 0x8D, 0x4C, 0x24, 0x0C, 0xE8, 0x18, 0x9C, 0x13, 0x00 },

//      SiMPLE:
//      mov ecx, DefaultExeCRC; mov dword ptr [edx], ecx; pop eax; pop eax
      { 0xB9, 0x64, 0xAC, 0x4A, 0xA1, 0x89, 0x0A, 0x58, 0x58 },

//      v1612:
//      xor ecx, ecx; inc ecx; mov dword ptr [edx], ecx; pop eax; pop eax
//      { 0x31, 0xC9, 0x41, 0x89, 0x0A, 0x58, 0x58, 0x90, 0x90 },

      0xD99DE,
      { 0x8D, 0x4D, 0xE4, 0xE8, 0x08, 0x39, 0x17, 0x00 },

//      I didn't find any requirements for
//      imm32 alignment, possibly there aren't any.
//      In debug executable, the imm32 operand lands
//      precicely at 4-byte boundary, so we're safe.
//      If the same patch is applied to main executable,
//      the imm32 would be misaligned. May be the reason
//      why SiMPLE used a longer instruction sequence

//      mov dword ptr [edx], DefaultExeCRC; pop eax; pop eax
      { 0xC7, 0x02, 0x64, 0xAC, 0x4A, 0xA1, 0x58, 0x58 }

//      v1.612:
//      xor ecx, ecx; inc ecx; mov dword ptr [edx], ecx; pop eax; pop eax
//      { 0x31, 0xC9, 0x41, 0x89, 0x0A, 0x58, 0x58, 0x90 }
    );


//    GameClient::AssembleChallengeResponse
//      use server-approved value for executable CRC32
    ApplyPatch(
      file, mOperationMode, exeType,

      0x95062,
      { 0x8D, 0x8C, 0x24, 0x70, 0x02, 0x00, 0x00, 0xE8, 0x12, 0xC2, 0x12, 0x00 },

//      SiMPLE:
//      mov ecx, DefaultExeCRC; mov dword ptr [edx], ecx; add esp, 8; xor eax, eax
      { 0xB9, 0x64, 0xAC, 0x4A, 0xA1, 0x89, 0x0A, 0x83, 0xC4, 0x08, 0x31, 0xC0 },

//      v1.612:
//      xor ecx, ecx; inc ecx; mov dword ptr [edx], ecx; add esp, 8; xor eax, eax
//      { 0x31, 0xC9, 0x41, 0x89, 0x0A, 0x83, 0xC4, 0x08, 0x31, 0xC0, 0x90, 0x90 },

      0xEB806,
      { 0x8D, 0x8D, 0x68, 0xF6, 0xFF, 0xFF, 0xE8, 0xDD, 0x1A, 0x16, 0x00 },

//      mov dword ptr [edx], DefaultExeCRC; add esp, 8; xor eax, eax
      { 0xC7, 0x02, 0x64, 0xAC, 0x4A, 0xA1, 0x83, 0xC4, 0x08, 0x31, 0xC0 }

//      v1.612:
//      xor ecx, ecx; inc ecx; mov dword ptr [edx], ecx; add esp, 8; xor eax, eax
//      { 0x31, 0xC9, 0x41, 0x89, 0x0A, 0x83, 0xC4, 0x08, 0x31, 0xC0, 0x90 }
    );

//    GameClient::AssembleChallengeResponse
//      use server-approved value for executable CRC32 (v1162 & SiMPLE WWII anthology)
    ApplyPatch(
      file,
//      mOperationMode, // v1.612
      OperationMode::Restore, // Not present in executable(s) from SiMPLE
      exeType,

      0x950B9,
      { 0x8B, 0x94, 0x24, 0x65, 0x04, 0x00, 0x00 },

//      mov edx, DefaultExeCRC
      { 0xBA, 0x64, 0xAC, 0x4A, 0xA1, 0x90, 0x90 },

      0xEB854,
      { 0x31, 0x8D, 0xE5, 0xF5, 0xFF, 0xFF },
      { /* Don't know how to fit it yet */ } );


//  in case we have a user-defined exe CRC
    if ( mOperationMode == OperationMode::Modify )
    {
      ApplyPatch(
        file, exeType,
        0x87660, 0xD99E0,
        ToBytes(mExeCrc) );

      ApplyPatch(
        file, exeType,
        0x95063, 0xEB808,
        ToBytes(mExeCrc) );

      ApplyPatch(
        file, 0x950BA,
        ToBytes(mExeCrc) );

//      Not present in executable(s) from SiMPLE
      if ( false )
        ApplyPatch(
          file, exeType,
          0x950BA, 0, // TODO: devise a patch
          ToBytes(mExeCrc) );
    }


//    GameClient::AssembleChallengeResponse
//      pretend bf1942 CD-key is present in the registry
    ApplyPatch(
      file, mOperationMode, exeType,
      0x9510A, 0xEB88E,
      { 0x75 },
      { 0xEB } );

//    Join/Internet/JoinInternetGame
//      ignore CD:\\setup.ini: Startup/ProductGUID check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x29FA9C,
      0x376839, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    Join/Internet/JoinInternetGame
//      ignore CD:\\bfdist.vlu check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x29FACD,
      0x376886, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    Join/Lan/JoinLanGame
//      ignore CD:\\setup.ini: Startup/ProductGUID check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2A4DD5,
      0x37B764, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    Join/Lan/JoinLanGame
//      ignore CD:\\bfdist.vlu check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2A4E06,
      0x37B7B1, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    hostServer
//    CreateGameMenu::HostLanGame
//    CreateGameMenu::HostInternetGame
//      ignore CD:\\setup.ini: Startup/ProductGUID check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2C0DD2,
      0x3986CB, // optional for debug exe
      { 0x0F, 0x85 },
      { 0x90, 0xE9 } );

//    Skirmish/StartSkirmish
//      ignore CD:\\setup.ini: Startup/ProductGUID check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2DCFCF,
      0x3B189B, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    CustomGame/ActivateCustomGame (dedicated server)
//      ignore CD:\\setup.ini: Startup/ProductGUID check
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2E832A,
      { 0x14 },
      { 0x00 },
      0x3BCF23, // optional for debug exe
      { 0x75 },
      { 0xEB } );

//    CustomGame/ActivateCustomGame (dedicated server)
//      check CD:\\bfdist.vlu
    ApplyPatch(
      file, mOperationMode, exeType,
      0x2E833D,
      0x3BCF74, // optional for debug exe
      { 0x75 },
      { 0xEB } );
  }
};

} // namespace Patches


enum ResultCode : int8_t
{
  Success,
  ReadFileFailed,
  WriteFileFailed,
  InvalidExecutableSize,
  PatchFailed,
};


struct BinaryFile
{
  enum OpenModeFlags : int
  {
    Read = 0b01,
    Write = 0b10,
    Truncate = 0b100,
  };

  std::string name {};
  int flags {}; // OpenModeFlags

  std::fstream file {};


  ResultCode open();
  void close();
};

ResultCode
BinaryFile::open()
{
  std::ios::openmode mode = std::ios::binary;
  std::string modeStr {};

  if ( flags & OpenModeFlags::Read )
  {
    mode |= std::ios::in;
    modeStr += "reading";
  }

  if ( flags & OpenModeFlags::Write )
  {
    mode |= std::ios::out;

    if ( flags & OpenModeFlags::Read )
      modeStr += " & ";

    modeStr += "writing";

    if ( flags & OpenModeFlags::Truncate )
    {
      mode |= std::ios::trunc;
      modeStr += "(truncate)";
    }
  }


  if ( file.is_open() == true )
    close();

  std::cout << "Opening '" << name << "'...";

  file.open(name, mode);


  if ( file.is_open() == false )
  {
    if ( modeStr.empty() == false )
      modeStr = " for " + modeStr;

    std::cout << "Failure\n";
    std::cout << "ERROR: Failed to open '" + name + "'" + modeStr << "\n\n";
    return ResultCode::ReadFileFailed;
  }

  file.exceptions(
    file.exceptions() |
    std::fstream::failbit ); // std::ios_base::failbit

  std::cout << "Success\n";

  return ResultCode::Success;
}

void
BinaryFile::close()
{
  file = {};
  flags = {};
}

ResultCode
VerifyExecutableSize(
  BinaryFile& exeFile,
  const size_t expectedSize,
  bool keepFileOpen = false )
{
  exeFile.flags = BinaryFile::Read;

  if (  auto result = exeFile.open();
        result != ResultCode::Success )
    return result;


  std::cout << "Verifying '" << exeFile.name << "' size...";

  std::ios::pos_type exeSize {};

  try
  {
    exeSize = exeFile.file.tellg();
    exeFile.file.seekg(0, std::ios::end);
    exeSize = exeFile.file.tellg() - exeSize;
  }
  catch ( const std::exception& e )
  {
    exeFile.close();
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    return ResultCode::ReadFileFailed;
  }

  if ( keepFileOpen == false )
    exeFile.close();


  if ( exeSize != expectedSize )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: this program expects executable size of "
              << expectedSize << " bytes. '"
              << exeFile.name + "' is " << exeSize << " bytes\n\n";
    return ResultCode::InvalidExecutableSize;
  }

  std::cout << "Success\n\n";

  return ResultCode::Success;
}


ResultCode
GenerateBlackScreen_r()
{
  BinaryFile blackScreen
  {
    blackScreenExeName[ExecutableType::Main],
    BinaryFile::Read,
  };

  if (  auto result = VerifyExecutableSize(blackScreen, 6144, true);
        result != ResultCode::Success )
    return result;


  BinaryFile blackScreen_r
  {
    blackScreenExeName[ExecutableType::Debug],
    BinaryFile::Write,
  };

  if (  auto result = blackScreen_r.open();
        result != ResultCode::Success )
    return result;


  ResultCode result {};

  try
  {
    std::cout << "Generating '" << blackScreen_r.name << "'...";

    blackScreen_r.file.seekp(0, std::ios::beg);
    blackScreen.file.seekp(0, std::ios::beg);
    blackScreen_r.file << blackScreen.file.rdbuf();
    blackScreen.close();

    std::cout << "Success\n";
  }
  catch ( const std::exception& e )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    return ResultCode::PatchFailed;
  }

  try
  {
    std::cout << "Patching '" << blackScreen_r.name << "'...";

    Patches::ApplyPatch(
      blackScreen_r.file, 0xF88,
      Patches::ToBytes(executableName[ExecutableType::Debug] + " %s") );

    std::cout << "Success\n\n";
  }
  catch ( const std::exception& e )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    result = ResultCode::PatchFailed;
  }

  return result;
}


void
waitForUserExit()
{
  std::cout << "Press ENTER to exit...\n";
  std::cin.get();
}


int
main(
  int,
  char** )
{
  while ( true )
  {
    std::cout << "Select operation (1/2/3):" << "\n"
              << "  1. Patch " << executableName[ExecutableType::Main] << " (default)" << "\n"
              << "  2. Patch " << executableName[ExecutableType::Debug] << "\n"
              << "  3. Generate " << blackScreenExeName[ExecutableType::Debug] << "\n";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if (  userInput.empty() == true ||
          userInput == "1" )
    {
      exeType = ExecutableType::Main;
      break;
    }

    if ( userInput == "2" )
    {
      exeType = ExecutableType::Debug;
      break;
    }

    if ( userInput == "3" )
    {
      auto result = GenerateBlackScreen_r();
      waitForUserExit();
      return result;
    }

    std::cout << "ERROR: Unknown command '" << userInput << "'\n\n";
  }

  std::cout << "\n";


  BinaryFile executable
  {
    executableName[exeType],
  };

  if (  auto result = VerifyExecutableSize(executable, executableSize[exeType]);
        result != ResultCode::Success )
  {
    waitForUserExit();
    return result;
  }


  using Patches::Patch;

  Patch* patches[] =
  {
    new Patches::Handle4GBAddressSpace(),
    new Patches::DefaultWindowPosition(),
    new Patches::ForceMaximizedWindow(),
    new Patches::DefaultWindowResolution(),
    new Patches::DontSetDefaultVideoMode(),
    new Patches::FixAspectRatio(),
    new Patches::InhibitAddToBuddyListFailedMessage(),
    new Patches::CustomMasterServer(),
    new Patches::CustomWindowTitle(),
    new Patches::MaxBotCount(),
    new Patches::InternetGamesSupport(),
    new Patches::MakePortable(),
  };

  auto deinitPatches =
  [&patches] ()
  {
    for ( auto patch : patches )
      delete patch;
  };


  for ( auto patch : patches )
    patch->promptUser(exeType);


  executable.flags = BinaryFile::Read | BinaryFile::Write;

  if (  auto result = executable.open();
        result != ResultCode::Success )
  {
    deinitPatches();
    waitForUserExit();

    return result;
  }


  ResultCode result {};

  try
  {
    std::cout << "Patching '" << executable.name << "'...";

    for ( auto patch : patches )
      patch->patch(executable.file, exeType);

    std::cout << "Success\n\n";
  }
  catch ( const std::exception& e )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    result = ResultCode::PatchFailed;
  }

  executable.close();
  deinitPatches();

  waitForUserExit();

  return result;
}
