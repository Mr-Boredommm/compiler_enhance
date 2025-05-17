
// Generated from MiniC.g4 by ANTLR 4.12.0


#include "MiniCLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct MiniCLexerStaticData final {
  MiniCLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MiniCLexerStaticData(const MiniCLexerStaticData&) = delete;
  MiniCLexerStaticData(MiniCLexerStaticData&&) = delete;
  MiniCLexerStaticData& operator=(const MiniCLexerStaticData&) = delete;
  MiniCLexerStaticData& operator=(MiniCLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag miniclexerLexerOnceFlag;
MiniCLexerStaticData *miniclexerLexerStaticData = nullptr;

void miniclexerLexerInitialize() {
  assert(miniclexerLexerStaticData == nullptr);
  auto staticData = std::make_unique<MiniCLexerStaticData>(
    std::vector<std::string>{
      "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", "T_R_BRACE", 
      "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_MUL", "T_DIV", "T_MOD", 
      "T_LT", "T_GT", "T_LE", "T_GE", "T_EQ", "T_NE", "T_AND", "T_OR", "T_NOT", 
      "T_IF", "T_ELSE", "T_WHILE", "T_BREAK", "T_CONTINUE", "T_RETURN", 
      "T_INT", "T_VOID", "T_ID", "T_DIGIT_LL", "T_DIGIT", "LINE_COMMENT", 
      "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "';'", "'{'", "'}'", "'='", "','", "'+'", "'-'", 
      "'*'", "'/'", "'%'", "'<'", "'>'", "'<='", "'>='", "'=='", "'!='", 
      "'&&'", "'||'", "'!'", "'if'", "'else'", "'while'", "'break'", "'continue'", 
      "'return'", "'int'", "'void'"
    },
    std::vector<std::string>{
      "", "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", "T_R_BRACE", 
      "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_MUL", "T_DIV", "T_MOD", 
      "T_LT", "T_GT", "T_LE", "T_GE", "T_EQ", "T_NE", "T_AND", "T_OR", "T_NOT", 
      "T_IF", "T_ELSE", "T_WHILE", "T_BREAK", "T_CONTINUE", "T_RETURN", 
      "T_INT", "T_VOID", "T_ID", "T_DIGIT_LL", "T_DIGIT", "LINE_COMMENT", 
      "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,34,235,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,1,0,1,0,1,1,1,
  	1,1,2,1,2,1,3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,
  	1,10,1,11,1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,14,1,15,1,15,1,15,1,16,
  	1,16,1,16,1,17,1,17,1,17,1,18,1,18,1,18,1,19,1,19,1,19,1,20,1,20,1,21,
  	1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,23,1,23,1,23,1,24,
  	1,24,1,24,1,24,1,24,1,24,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,
  	1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,27,1,27,1,27,1,27,1,28,1,28,1,28,
  	1,28,1,28,1,29,1,29,5,29,165,8,29,10,29,12,29,168,9,29,1,30,1,30,1,30,
  	4,30,173,8,30,11,30,12,30,174,1,30,1,30,1,30,4,30,180,8,30,11,30,12,30,
  	181,1,30,1,30,1,30,5,30,187,8,30,10,30,12,30,190,9,30,1,30,3,30,193,8,
  	30,1,31,1,31,1,31,4,31,198,8,31,11,31,12,31,199,1,31,1,31,4,31,204,8,
  	31,11,31,12,31,205,1,31,1,31,1,31,5,31,211,8,31,10,31,12,31,214,9,31,
  	3,31,216,8,31,1,32,1,32,1,32,1,32,5,32,222,8,32,10,32,12,32,225,9,32,
  	1,32,1,32,1,33,4,33,230,8,33,11,33,12,33,231,1,33,1,33,0,0,34,1,1,3,2,
  	5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,
  	16,33,17,35,18,37,19,39,20,41,21,43,22,45,23,47,24,49,25,51,26,53,27,
  	55,28,57,29,59,30,61,31,63,32,65,33,67,34,1,0,10,3,0,65,90,95,95,97,122,
  	4,0,48,57,65,90,95,95,97,122,2,0,88,88,120,120,3,0,48,57,65,70,97,102,
  	2,0,76,76,108,108,1,0,48,55,1,0,49,57,1,0,48,57,2,0,10,10,13,13,3,0,9,
  	10,13,13,32,32,248,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,
  	9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,
  	0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,
  	0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,0,0,0,
  	41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,1,0,0,0,0,49,1,0,0,0,0,51,1,
  	0,0,0,0,53,1,0,0,0,0,55,1,0,0,0,0,57,1,0,0,0,0,59,1,0,0,0,0,61,1,0,0,
  	0,0,63,1,0,0,0,0,65,1,0,0,0,0,67,1,0,0,0,1,69,1,0,0,0,3,71,1,0,0,0,5,
  	73,1,0,0,0,7,75,1,0,0,0,9,77,1,0,0,0,11,79,1,0,0,0,13,81,1,0,0,0,15,83,
  	1,0,0,0,17,85,1,0,0,0,19,87,1,0,0,0,21,89,1,0,0,0,23,91,1,0,0,0,25,93,
  	1,0,0,0,27,95,1,0,0,0,29,97,1,0,0,0,31,100,1,0,0,0,33,103,1,0,0,0,35,
  	106,1,0,0,0,37,109,1,0,0,0,39,112,1,0,0,0,41,115,1,0,0,0,43,117,1,0,0,
  	0,45,120,1,0,0,0,47,125,1,0,0,0,49,131,1,0,0,0,51,137,1,0,0,0,53,146,
  	1,0,0,0,55,153,1,0,0,0,57,157,1,0,0,0,59,162,1,0,0,0,61,192,1,0,0,0,63,
  	215,1,0,0,0,65,217,1,0,0,0,67,229,1,0,0,0,69,70,5,40,0,0,70,2,1,0,0,0,
  	71,72,5,41,0,0,72,4,1,0,0,0,73,74,5,59,0,0,74,6,1,0,0,0,75,76,5,123,0,
  	0,76,8,1,0,0,0,77,78,5,125,0,0,78,10,1,0,0,0,79,80,5,61,0,0,80,12,1,0,
  	0,0,81,82,5,44,0,0,82,14,1,0,0,0,83,84,5,43,0,0,84,16,1,0,0,0,85,86,5,
  	45,0,0,86,18,1,0,0,0,87,88,5,42,0,0,88,20,1,0,0,0,89,90,5,47,0,0,90,22,
  	1,0,0,0,91,92,5,37,0,0,92,24,1,0,0,0,93,94,5,60,0,0,94,26,1,0,0,0,95,
  	96,5,62,0,0,96,28,1,0,0,0,97,98,5,60,0,0,98,99,5,61,0,0,99,30,1,0,0,0,
  	100,101,5,62,0,0,101,102,5,61,0,0,102,32,1,0,0,0,103,104,5,61,0,0,104,
  	105,5,61,0,0,105,34,1,0,0,0,106,107,5,33,0,0,107,108,5,61,0,0,108,36,
  	1,0,0,0,109,110,5,38,0,0,110,111,5,38,0,0,111,38,1,0,0,0,112,113,5,124,
  	0,0,113,114,5,124,0,0,114,40,1,0,0,0,115,116,5,33,0,0,116,42,1,0,0,0,
  	117,118,5,105,0,0,118,119,5,102,0,0,119,44,1,0,0,0,120,121,5,101,0,0,
  	121,122,5,108,0,0,122,123,5,115,0,0,123,124,5,101,0,0,124,46,1,0,0,0,
  	125,126,5,119,0,0,126,127,5,104,0,0,127,128,5,105,0,0,128,129,5,108,0,
  	0,129,130,5,101,0,0,130,48,1,0,0,0,131,132,5,98,0,0,132,133,5,114,0,0,
  	133,134,5,101,0,0,134,135,5,97,0,0,135,136,5,107,0,0,136,50,1,0,0,0,137,
  	138,5,99,0,0,138,139,5,111,0,0,139,140,5,110,0,0,140,141,5,116,0,0,141,
  	142,5,105,0,0,142,143,5,110,0,0,143,144,5,117,0,0,144,145,5,101,0,0,145,
  	52,1,0,0,0,146,147,5,114,0,0,147,148,5,101,0,0,148,149,5,116,0,0,149,
  	150,5,117,0,0,150,151,5,114,0,0,151,152,5,110,0,0,152,54,1,0,0,0,153,
  	154,5,105,0,0,154,155,5,110,0,0,155,156,5,116,0,0,156,56,1,0,0,0,157,
  	158,5,118,0,0,158,159,5,111,0,0,159,160,5,105,0,0,160,161,5,100,0,0,161,
  	58,1,0,0,0,162,166,7,0,0,0,163,165,7,1,0,0,164,163,1,0,0,0,165,168,1,
  	0,0,0,166,164,1,0,0,0,166,167,1,0,0,0,167,60,1,0,0,0,168,166,1,0,0,0,
  	169,170,5,48,0,0,170,172,7,2,0,0,171,173,7,3,0,0,172,171,1,0,0,0,173,
  	174,1,0,0,0,174,172,1,0,0,0,174,175,1,0,0,0,175,176,1,0,0,0,176,193,7,
  	4,0,0,177,179,5,48,0,0,178,180,7,5,0,0,179,178,1,0,0,0,180,181,1,0,0,
  	0,181,179,1,0,0,0,181,182,1,0,0,0,182,183,1,0,0,0,183,193,7,4,0,0,184,
  	188,7,6,0,0,185,187,7,7,0,0,186,185,1,0,0,0,187,190,1,0,0,0,188,186,1,
  	0,0,0,188,189,1,0,0,0,189,191,1,0,0,0,190,188,1,0,0,0,191,193,7,4,0,0,
  	192,169,1,0,0,0,192,177,1,0,0,0,192,184,1,0,0,0,193,62,1,0,0,0,194,195,
  	5,48,0,0,195,197,7,2,0,0,196,198,7,3,0,0,197,196,1,0,0,0,198,199,1,0,
  	0,0,199,197,1,0,0,0,199,200,1,0,0,0,200,216,1,0,0,0,201,203,5,48,0,0,
  	202,204,7,5,0,0,203,202,1,0,0,0,204,205,1,0,0,0,205,203,1,0,0,0,205,206,
  	1,0,0,0,206,216,1,0,0,0,207,216,5,48,0,0,208,212,7,6,0,0,209,211,7,7,
  	0,0,210,209,1,0,0,0,211,214,1,0,0,0,212,210,1,0,0,0,212,213,1,0,0,0,213,
  	216,1,0,0,0,214,212,1,0,0,0,215,194,1,0,0,0,215,201,1,0,0,0,215,207,1,
  	0,0,0,215,208,1,0,0,0,216,64,1,0,0,0,217,218,5,47,0,0,218,219,5,47,0,
  	0,219,223,1,0,0,0,220,222,8,8,0,0,221,220,1,0,0,0,222,225,1,0,0,0,223,
  	221,1,0,0,0,223,224,1,0,0,0,224,226,1,0,0,0,225,223,1,0,0,0,226,227,6,
  	32,0,0,227,66,1,0,0,0,228,230,7,9,0,0,229,228,1,0,0,0,230,231,1,0,0,0,
  	231,229,1,0,0,0,231,232,1,0,0,0,232,233,1,0,0,0,233,234,6,33,0,0,234,
  	68,1,0,0,0,12,0,166,174,181,188,192,199,205,212,215,223,231,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  miniclexerLexerStaticData = staticData.release();
}

}

MiniCLexer::MiniCLexer(CharStream *input) : Lexer(input) {
  MiniCLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *miniclexerLexerStaticData->atn, miniclexerLexerStaticData->decisionToDFA, miniclexerLexerStaticData->sharedContextCache);
}

MiniCLexer::~MiniCLexer() {
  delete _interpreter;
}

std::string MiniCLexer::getGrammarFileName() const {
  return "MiniC.g4";
}

const std::vector<std::string>& MiniCLexer::getRuleNames() const {
  return miniclexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& MiniCLexer::getChannelNames() const {
  return miniclexerLexerStaticData->channelNames;
}

const std::vector<std::string>& MiniCLexer::getModeNames() const {
  return miniclexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& MiniCLexer::getVocabulary() const {
  return miniclexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MiniCLexer::getSerializedATN() const {
  return miniclexerLexerStaticData->serializedATN;
}

const atn::ATN& MiniCLexer::getATN() const {
  return *miniclexerLexerStaticData->atn;
}




void MiniCLexer::initialize() {
  ::antlr4::internal::call_once(miniclexerLexerOnceFlag, miniclexerLexerInitialize);
}
