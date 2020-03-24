#include <stdbool.h>

#include "strread.h"

/**
 * @file fasta.h
 * @author Paulo Fonseca
 * @brief FASTA file sequence reader
 * 
 * The `fasta` reader provides a way to iterate through the sequences 
 * defined in a FASTA-format file. 
 */

typedef struct _fasta fasta;

/**
 * @brief A FASTA record with in-memory sequence
 */
typedef struct {
    char *descr; /**< Sequence descriptor (does not include the `>`) */
    char *seq;   /**< In-memory sequence content */ 
} fasta_record;

/**
 * @brief A FASTA record with sequence loaded as a (inner) stream
 */
typedef struct {
    char *descr;      /**< Sequence descriptor */
    strread *seqread; /**< Sequence contents reader */
} fasta_record_reader;


/**
 * @brief Opens a FASTA file and places the cursor at the beginning 
 */
fasta *fasta_open(char *filename);


/**
 * @brief Checks whether there is a sequence *after* the current 
 * cursor position
 */
bool fasta_has_next(fasta *self);


/**
 * @brief Advances the cursor to the next sequence record, and loads 
 * it as a stream.
 * The returned strread reader is a virtual stream that points to the 
 * beginning of the sequence contents inside the file, and is used 
 * to read these contents as if they were in a separate stream, 
 * ignoring any line breaks. EOF is reached at the end of the sequence.
 * 
 * Example 
 * -------
 * 
 * ```C
 * fasta *fr = fasta_open("teste.fa");
 * while(fasta_has_next(fr)) {
 *  fasta_record_reader rec = fasta_next_reader(fr);
 *  for (xchar c; (c=strread_getc(rec->seqread)) != EOF;) {
 *      //do something with c 
 *  }
 * }
 * ```
 * @warning The returned record reader object contains inner referenced and 
 * should *not* be destroyed directly.
 * @see strread
 */
fasta_record_reader *fasta_next_reader(fasta *self);


/**
 * @brief Closes the reader and releases used resources
 */
void fasta_close(fasta *self);