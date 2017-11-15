#ifndef __STRING_TOKENIZER_H_V2_
#define __STRING_TOKENIZER_H_V2_

#include <vector>
#include <stdexcept>
#include <string.h>

namespace terra
{
   namespace common
   {
      template<int MAX_SIZE>
      class string_tokenizer
      {
      public:
         /** Each token is a separate ptr to our own managed string */
         typedef std::vector<char*> token_vector;

         // Breaks a line into tokens separated by 'separator'.
         // Returns the quantity of tokens found. This qty
         // is always greater than 0 (see samples above)
         size_t break_line(const char* line, char separator);
         size_t break_line(const char* line, const char* separator);

         //  Reads the current token and position on the next
         const char* next();

         // Indicates if end was reached
         bool end() const;

         // Reposition on the first token
         void reposition();

         // Remove empty tokens
         void remove_empty_tokens();

         // Access a token by the position.
         // Throws an exception if the position is not valid
         const char* operator[](size_t index) const;

         // Returns the quantity of tokens. This qty
         // is always greater than 0 (see samples above)
         size_t size() const;

         // Returns true if the token on 'index' is an empty string
         // or if the 'index' is out of range.
         bool is_empty(size_t index) const;

         // Returns true if the token on 'index' is equal to 'str'.
         // Throws an exception if the position is not valid
         bool is_equal(size_t index, const char* str) const;

         // Returns the size of token on 'index'
         // Throws an exception if the position is not valid
         size_t get_size(size_t index) const;

      private:
         /** vector of tokens */
         token_vector m_tokens;

         /** Current token position */
         token_vector::iterator m_pos;

         /** Our own memory managed "string" */
         char m_buf[MAX_SIZE+1];
      };


      // Inline methods
      template <int MAX_SIZE>
      inline bool string_tokenizer<MAX_SIZE>::end() const
      {
         return m_pos == m_tokens.end();
      }

      template <int MAX_SIZE>
      inline void string_tokenizer<MAX_SIZE>::reposition()
      {
         m_pos = m_tokens.begin();
      }

      template <int MAX_SIZE>
      inline size_t string_tokenizer<MAX_SIZE>::size() const
      {
         return m_tokens.size();
      }

      //----------------------------------------------------------------
      // break_line
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      size_t string_tokenizer<MAX_SIZE>::break_line(const char* line, char separator)
      {
         if (strlen(line) > MAX_SIZE)
            throw std::length_error("Line is too large.");

         strncpy(m_buf, line, MAX_SIZE);
         m_buf[MAX_SIZE] = 0;
         m_tokens.clear();

         if (strlen(m_buf) == 0) // empty line: just one empty string
         {
            m_tokens.push_back(m_buf);
            *m_buf = 0;
         }
         else
         {
            char* cursor = m_buf; // traverse on line
            char* mark = m_buf; // points to a token begin
            char* lineEnd = m_buf + strlen(m_buf); // points to line end

            while (cursor != lineEnd)
            {
               if (*cursor == separator) // token found
               {
                  m_tokens.push_back(mark);
                  *cursor = 0;
                  mark = cursor + 1; //-- Points to the beginning of next token
               }
               ++cursor; 
            }

            if (mark != lineEnd)
               m_tokens.push_back(mark); // The last token
            else
            {
               // Last char is a separator: add an empty string token
               if (*(line + strlen(line) - 1) == separator) 
                  m_tokens.push_back(lineEnd);
            }
         }
         reposition(); // Position on m_tokens's begin
         return m_tokens.size(); // Number of tokens found
      }

      template <int MAX_SIZE>
      size_t string_tokenizer<MAX_SIZE>::break_line(const char* line, const char* separator)
      {
         if (strlen(line) > MAX_SIZE)
            throw std::length_error("Line is too large.");

         strncpy(m_buf, line, MAX_SIZE);
         m_buf[MAX_SIZE] = 0;
         m_tokens.clear();

         if (strlen(m_buf) == 0) // empty line: just one empty string
         {
            m_tokens.push_back(m_buf);
            *m_buf = 0;
         }
         else
         {
            char* cursor = m_buf; // traverse on line
            char* mark = m_buf; // points to a token begin
            char* lineEnd = m_buf + strlen(m_buf); // points to line end

            while (cursor != lineEnd)
            {
               //if (*cursor == separator) // token found
               if (strchr(separator, *cursor) != NULL) // token found
               {
                  m_tokens.push_back(mark);
                  *cursor = 0;
                  mark = cursor + 1; //-- Points to the beginning of next token
               }
               ++cursor; 
            }

            if (mark != lineEnd)
               m_tokens.push_back(mark); // The last token
            else
            {
               // Last char is a separator: add an empty string token
               //if (*(line + strlen(line) - 1) == separator) 
               if (strchr(separator, *(line + strlen(line) - 1)) != NULL)
                  m_tokens.push_back(lineEnd);
            }
         }
         reposition(); // Position on m_tokens's begin
         return m_tokens.size(); // Number of tokens found
      }

      //----------------------------------------------------------------
      // next
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      const char* string_tokenizer<MAX_SIZE>::next()
      {
         if (m_pos == m_tokens.end())
            throw std::logic_error("Trying to read after end");

         return *m_pos++; // Returns the current and skips to the next
      }

      //----------------------------------------------------------------
      // operator[]
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      const char* string_tokenizer<MAX_SIZE>::operator[](size_t index) const
      {
         if (index >= m_tokens.size())
            throw std::logic_error("Invalid index");
         return m_tokens[index];
      }

      //----------------------------------------------------------------
      // is_empty
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      bool string_tokenizer<MAX_SIZE>::is_empty(size_t index) const
      {
         if (index >= m_tokens.size())
            return true;
         return strlen(m_tokens[index]) == 0;
      }
      //----------------------------------------------------------------
      // is_equal
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      bool string_tokenizer<MAX_SIZE>::is_equal(size_t index, const char* str) const
      {
         if (index >= m_tokens.size())
            throw std::logic_error("Invalid index");
         return strcmp(m_tokens[index], str) == 0;
      }
      //----------------------------------------------------------------
      // get_size
      //----------------------------------------------------------------
      template <int MAX_SIZE>
      size_t string_tokenizer<MAX_SIZE>::get_size(size_t index) const
      {
         if (index >= m_tokens.size())
            throw std::logic_error("Invalid index");
         return strlen(m_tokens[index]);
      }

      template <int MAX_SIZE>
      void string_tokenizer<MAX_SIZE>::remove_empty_tokens()
      {
         token_vector::iterator it = m_tokens.begin();
         while (it < m_tokens.end())
         {
            if (strlen(*it) == 0)
               it = m_tokens.erase(it);
            else
               it++;
         }
         reposition();
      }
   }
}
#endif // __STRING_TOKENIZER_H__
