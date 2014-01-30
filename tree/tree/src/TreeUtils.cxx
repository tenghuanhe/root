// @(#)root/tree:$Id$
// Author: Timur Pocheptsov   30/01/2014

/*************************************************************************
 * Copyright (C) 1995-2014, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TreeUtils                                                            //
//                                                                      //
// Different standalone functions to work with trees and tuples,        //
// not reqiuired to be a member of any class.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <istream>
#include <cassert>
#include <cctype>

#include "TreeUtils.h"
#include "TNtupleD.h"
#include "TNtuple.h"
#include "TError.h"
#include "TTree.h"

namespace ROOT {
namespace TreeUtils {

//Some aux. functions to read tuple from a text file. No reason to make them memeber-functions.
void SkipEmptyLines(std::istream &input);
void SkipWSCharacters(std::istream &input);
bool NextCharacterIsEOL(std::istream &input);

//Let's enforce/limit what can be a Tuple.
//Actually, at the moment you can only use
//the fill function only for TNtuple/TNtupleD
//(enforced by hidden definition and explicit instantiations).
//But in future this can potentially change.

template<class> struct InvalidTupleType;

template<>
struct InvalidTupleType<TNtuple>
{
   InvalidTupleType()
   {
   }
};

template<>
struct InvalidTupleType<TNtupleD>
{
   InvalidTupleType()
   {
   }
};

//_______________________________________________________________________
template<class DataType, class Tuple>
Long64_t FillNtupleFromStream(std::istream &inputStream, Tuple &tuple, char delimiter, bool relaxedMode)
{
   InvalidTupleType<Tuple> typeChecker;

   if (delimiter == '\r' || delimiter == '\n') {
      ::Error("FillNtupleFromStream", "invalid delimiter - newline character");
      return 0;
   }

   const Int_t nVars = tuple.GetNvar();
   if (!nVars) {
      ::Error("FillNtupleFromStream", "invalid number of elements");
      return 0;
   }
   
   DataType *args = tuple.GetArgs();
   assert(args != 0 && "FillNtupleFromStream, args buffer is a null");
   
   Long64_t nLines = 0;
   
   if (!relaxedMode) {
      while (true) {
         //Skip empty-lines (containing only newlines, comments, whitespaces + newlines
         //and combinations).
         SkipEmptyLines(inputStream);
         
         if (!inputStream.good()) {
            if (!nLines)
               ::Error("FillNtupleFromStream", "no data read");
            return nLines;
         }

         //Now, we have to be able to read _the_ _required_ number of entires.
         for (Int_t i = 0; i < nVars; ++i) {
            SkipWSCharacters(inputStream);//skip all wses except newlines.
            if (!inputStream.good()) {
               ::Error("FillNtupleFromStream", "failed to read a tuple (invalid line)");
               return nLines;
            }

            if (i > 0 && !std::isspace(delimiter)) {
               const char test = inputStream.peek();
               if (!inputStream.good() || test != delimiter) {
                  ::Error("FillNtupleFromStream", "delimiter expected");
                  return nLines;
               }

               inputStream.get();//we skip a dilimiter whatever it is.
               SkipWSCharacters(inputStream);
            }
            
            if (NextCharacterIsEOL(inputStream)) {
               //This is unexpected!
               ::Error("FillNtupleFromStream", "unexpected character or eof found");
               return nLines;
            }

            inputStream>>args[i];
            
            if (!(inputStream.eof() && i + 1 == nVars) && !inputStream.good()){
               ::Error("FillNtupleFromStream", "error while reading a value");
               return nLines;
            }
         }
         
         //Only God forgives :) Ugly but ...
         //for TNtuple it's protected :)
         static_cast<TTree &>(tuple).Fill();
         ++nLines;
      }
   } else {
      //This was requested in JIRA: read values for a given row even if they are separated
      //by newline-character.
   }
   
   return nLines;
}

template Long64_t FillNtupleFromStream<Float_t, TNtuple>(std::istream &, TNtuple &, char, bool);
template Long64_t FillNtupleFromStream<Double_t, TNtupleD>(std::istream &, TNtupleD &, char, bool);

//Aux. functions to read tuples from text files.

//file:
//    lines
//
//lines:
//    line
//    line lines
//
//line:
//    comment
//    tuple
//    empty-line



//comment:
// '#' non-newline-character-sequence newline-character
//
//non-newline-character-sequence:
// any symbol except '\r' or '\n'
//
//newline-character:
// '\r' | '\n'
//______________________________________________________________________________
void SkipComment(std::istream &input)
{
   //Skips everything from '#' to (including) '\r' or '\n'.

   while (input.good()) {
      const char next = input.peek();
      if (input.good()) {
         input.get();
         if (next == '\r' || next == '\n')
            break;
      }
   }
}

//empty-line:
//    newline-character
//    ws-sequence newline-character
//    ws-sequence comment
//______________________________________________________________________________
void SkipEmptyLines(std::istream &input)
{
   //Skips empty lines (newline-characters), ws-lines (consisting only of whitespace characters + newline-characters).
   
   while (input.good()) {
      const char c = input.peek();
      if (!input.good())
         break;

      if (c == '#')
         SkipComment(input);
      else if (!std::isspace(c))//'\r' and '\n' are also 'isspaces'.
         break;
      else
         input.get();
   }
}

//ws-sequence:
//    c such that isspace(c) and c is not a newline-character.
//______________________________________________________________________________
void SkipWSCharacters(std::istream &input)
{
   //Skip whitespace characters, but not newline-characters we support ('\r' or '\n').
   while (input.good()) {
      const char next = input.peek();
      if (input.good()) {
         if (std::isspace(next) && next != '\n' && next != '\r')
            input.get();
         else
            break;
      }
   }
}

//Next character is either newline-character, eof or we have some problems reading
//the next symbol.
//______________________________________________________________________________
bool NextCharacterIsEOL(std::istream &input)
{
   //Either '\r' | '\n' or eof of some problem.
   if (!input.good())
      return true;
   
   const char next = input.peek();
   if (!input.good())
      return true;
  
   return next == '\r' || next == '\n';
}

}//TreeUtils
}//ROOT
