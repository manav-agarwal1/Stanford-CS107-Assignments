/**
 * File: rsg.cc
 * ------------
 * Provides the implementation of the full RSG application, which
 * relies on the services of the built-in string, ifstream, vector,
 * and map classes as well as the custom Production and Definition
 * classes provided with the assignment.
 */
 
#include <map>
#include <fstream>
#include "definition.h"
#include "production.h"
#include <assert.h>
using namespace std;

/**
 * Takes a reference to a legitimate infile (one that's been set up
 * to layer over a file) and populates the grammar map with the
 * collection of definitions that are spelled out in the referenced
 * file.  The function is written under the assumption that the
 * referenced data file is really a grammar file that's properly
 * formatted.  You may assume that all grammars are in fact properly
 * formatted.
 *
 * @param infile a valid reference to a flat text file storing the grammar.
 * @param grammar a reference to the STL map, which maps nonterminal strings
 *                to their definitions.
 */

static void readGrammar(ifstream& infile, map<string, Definition>& grammar)
{
  while (true) {
    string uselessText;
    getline(infile, uselessText, '{');
    if (infile.eof()) return;  // true? we encountered EOF before we saw a '{': no more productions!
    infile.putback('{'); // if file is write then we need to put back { as it got consumed and Definition expects {.
    Definition def(infile);
    grammar[def.getNonterminal()] = def;
  }
}

/**
 * @brief Generates Random production given a Grammar file.
 * enter at <start> and move its way forward
 * 
 */
void generateRandomSequence(map<string, Definition>& grammar, string& entry, vector<string>& result) {
  if (grammar.find(entry) == grammar.end()) {
    return;
  }
  Production randProduction = grammar[entry].getRandomProduction();
  for (Production::const_iterator curr = randProduction.begin(); curr != randProduction.end(); ++curr) {
    // cout << (*curr) << '\n';
    if ((*curr)[0] == '<') {
      string temp = (*curr);
      generateRandomSequence(grammar, temp, result);
    }
    else {
      result.push_back((*curr));
    }
  }
}

/**
 * Performs the rudimentary error checking needed to confirm that
 * the client provided a grammar file.  It then continues to
 * open the file, read the grammar into a map<string, Definition>,
 * and then print out the total number of Definitions that were read
 * in.  You're to update and decompose the main function to print
 * three randomly generated sentences, as illustrated by the sample
 * application.
 *
 * @param argc the number of tokens making up the command that invoked
 *             the RSG executable.  There must be at least two arguments,
 *             and only the first two are used.
 * @param argv the sequence of tokens making up the command, where each
 *             token is represented as a '\0'-terminated C string.
 */

int main(int argc, char *argv[])
{
  if (argc == 1) {
    cerr << "You need to specify the name of a grammar file." << endl;
    cerr << "Usage: rsg <path to grammar text file>" << endl;
    return 1; // non-zero return value means something bad happened 
  }
  
  ifstream grammarFile(argv[1]);
  if (grammarFile.fail()) {
    cerr << "Failed to open the file named \"" << argv[1] << "\".  Check to ensure the file exists. " << endl;
    return 2; // each bad thing has its own bad return value
  }
  
  // things are looking good...
  vector<string> result;
  map<string, Definition> grammar;
  readGrammar(grammarFile, grammar);
  string entry = "<start>";
  cout << "The grammar file called \"" << argv[1] << "\" contains "
       << grammar.size() << " definitions." << endl;
  // for (auto &it: grammar) {
  //   cout << "Non Terminal: " << it.first << '\n';
  //   auto vars = it.second.getAllProductions();
  //   for (auto &var : vars) {
  //     for (Production::iterator curr = var.begin(); curr != var.end(); ++curr) {
  //       cout << (*curr) << '\n';
  //     }
  //     cout << "--------------\n";
  //   }
  //   cout << "@@@@@@@@@@@@@@@@@@@@@@\n";
  // }
  generateRandomSequence(grammar, entry, result);
  int count = 0;
  for (string& s: result) {
    if (count == 0) {
      cout << "\t" << s;
    }
    else {
      if ((int) s.size() > 1) {
        cout << ' ' << s;
      }
      else {
        cout << s;
      }
    }
    count += (int) s.size();
    if (count >= 30) {
      cout << '\n';
      count = 0;
    }
  }
  cout << '\n';
  return 0;
}
