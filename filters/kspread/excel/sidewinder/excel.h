/* Sidewinder - Portable library for spreadsheet
   Copyright (C) 2003 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, US
*/

#ifndef SIDEWINDER_EXCEL_H
#define SIDEWINDER_EXCEL_H

#include <string>
#include <iostream>

#include "sidewinder.h"

namespace Sidewinder
{

/**
    Supported Excel document version.
*/
enum { UnknownExcel = 0, Excel95, Excel97, Excel2000 };

class Record;

// rich-text, unicode, far-east support Excel string

class EString
{
public:

  EString();
  
  EString( const EString& );
  
  EString& operator=( const EString& );
  
  ~EString();
  
  bool unicode() const;
  
  void setUnicode( bool u );
  
  bool richText() const;
  
  void setRichText( bool r );
    
  UString str() const;
  
  void setStr( const UString& str );
  
  // space allocate for the string, not length (use string.length() for that) 
  unsigned size() const;
  void setSize( unsigned size ); // HACKS
  
  
  static EString fromUnicodeString( const void* p, unsigned maxsize = 0 );
  
  static EString fromByteString( const void* p, unsigned maxsize = 0 );
  
  static EString fromSheetName( const void* p, unsigned maxsize = 0 );
  
  // from the buffer
  // longstring means 16-bit string length, usually for label
  // longstring=false is normally for sheet name
  static EString fromByteString( const void* p, bool longString, unsigned maxsize = 0 );
  
private:
  class Private;
  Private* d;
};

class RecordFactory
{
public:
  static Record* create( unsigned type );
};

/**
  Class Record represents a base class for all other type record,
  hence do not use this class in real life.
   
 */
class Record
{
public:

  /**
    Static ID of the record. Subclasses should override this value
    with the id of the record they handle.
  */  
  static const unsigned int id;
  
  /**
    Creates a new generic record.
  */  
  Record();
  
  /**
    Destroys the record.
  */
  virtual ~Record();
  
  void setVersion( unsigned v ){ ver = v; };
  
  unsigned version(){ return ver; };

  /**
    Sets the data for this record.
   */
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
    Sets the position of the record in the OLE stream. Somehow this is
    required to process BoundSheet and BOF(Worksheet) properly.
   */
  void setPosition( unsigned pos );
  
  /**
    Gets the position of this record in the OLE stream. 
   */
  unsigned position() const;
  
  /**
    Returns the name of the record. For debugging only.
   */
  virtual const char* name(){ return "Unknown"; }

  /**
    Dumps record information to output stream. For debugging only.
   */
  virtual void dump( std::ostream& out ) const;
  
protected:

   // position of this record in the OLE stream
   unsigned stream_position;

   // in which version does this record denote ?
   unsigned ver;

private:
   // no copy or assign
   Record( const Record& );
   Record& operator=( const Record& );
   
};

/**
  Class CellInfo represents a base class for records which provide information
  about cells. Some important records like BlankRecord, LabelRecord, and others
  inherit this class.   
 */
class CellInfo
{
public:

  /**
   * Creates a new cell information.
   */
  CellInfo();

  /**
   * Destroys the cell information.
   */
  virtual ~CellInfo();

  /**
   * Returns the row associated with the cell information. It is zero based,
   * so the first row is 0.
   *
   * \sa setRow, column
   */
  virtual unsigned row() const;

  /**
   * Returns the column associated with the cell information. It is zero based,
   * so the first column is 0.
   *
   * \sa setColumn, row
   */
  virtual unsigned column() const;

  /**
   * Returns the XF index for formatting of the cell.
   *
   * \sa setXfIndex
   */
  virtual unsigned xfIndex() const;

  /**
   * Sets the row associated with the cell information. It is zero based,
   * so the first row is 0.
   *
   * \sa setColumn, row
   */
  virtual void setRow( unsigned r );

  /**
   * Sets the column associated with the cell information. It is zero based,
   * so the first column is 0.
   *
   * \sa setRow, column
   */
  virtual void setColumn( unsigned c );

  /**
   * Sets the XF index for formatting of the cell.
   *
   * \sa xfIndex
   */
  virtual void setXfIndex( unsigned i );

private:
   // no copy or assign
   CellInfo( const CellInfo& );
   CellInfo& operator=( const CellInfo& );

   class Private;
   Private* info;
};

/**
  Class CellInfo represents a base class for records which provide information
  about a span of columns. The information, like formatting index, should 
  apply to columns (as specified) from firstColumn and lastColumn.
 */
class ColumnSpanInfo
{
public:

  /**
   * Creates a new column span information.
   */
  ColumnSpanInfo();

  /**
   * Destroys the column span information.
   */
  virtual ~ColumnSpanInfo();

  /**
   * Returns the first column associated with the span information. 
   * Column index is zero based, so the first column is 0.
   *
   * \sa lastColumn, setFirstColumn
   */
  virtual unsigned firstColumn() const;

  /**
   * Returns the last column associated with the span information. 
   * Column index is zero based, so the first column is 0.
   *
   * \sa firstColumn, setLastColumn
   */
  virtual unsigned lastColumn() const;

  /**
   * Sets the first column associated with the span information. 
   * Column index is zero based, so the first column is 0.
   *
   * \sa setLastColumn, firstColumn
   */
  virtual void setFirstColumn( unsigned c );

  /**
   * Sets the last column associated with the span information. 
   * Column index is zero based, so the first column is 0.
   *
   * \sa setFirstColumn, lastColumn
   */
  virtual void setLastColumn( unsigned c );

private:
   // no copy or assign
   ColumnSpanInfo( const ColumnSpanInfo& );
   ColumnSpanInfo& operator=( const ColumnSpanInfo& );

   class Private;
   Private* spaninfo;
};

/**
  Class BOFRecord represents BOF (Beginning of File) record, which
  is used to mark beginning of a set of records (following the BOF record).
  For each BOF record, there should also be one corresponding EOF record.

  Note that the name "BOF" is rather misleading. This is because of
  historical reason only.

  \sa EOFRecord

 */
class BOFRecord : public Record
{
public:

  /**
    Static ID of the BOF record.
  */
  static const unsigned int id;

  /**
    Supported BOF type.
  */
  enum { UnknownType = 0, Workbook, Worksheet, Chart, VBModule };

  /**
   * Creates a new BOF record.
   */
  BOFRecord();

  /**
    Destroys the record.
  */
  virtual ~BOFRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
    Returns the version, like Excel95, Excel97, and so on.

    Note that it is possible to use expression like 'version() >= Excel97'
    but always do that carefully.
  */
  unsigned version() const;

  /**
    Returns the version as string, something like "Excel97".
  */
  const char* versionAsString() const;

  /**
    Returns type of the BOF record, like Workbook, Chart, and so on.
  */
  unsigned type() const;

  /**
    Returns BOF type as string, something like "Worksheet".
  */
  const char* typeAsString() const;

  /**
   \reimpl
   */
  virtual const char* name(){ return "BOF"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   BOFRecord( const BOFRecord& );
   BOFRecord& operator=( const BOFRecord& );

   class Private;
   Private *d;
};

/**
  Class BlankRecord represents a blank cell. It contains information
  about cell address and formatting.
 */
class BlankRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new Blank record.
   */
  BlankRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "BLANK"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   BlankRecord( const BlankRecord& );
   BlankRecord& operator=( const BlankRecord& );
};

/**
  Class BOFRecord represents BoolErr record, which
  is used to store boolean value or error code of a cell.
 */
class BoolErrRecord : public Record, public CellInfo
{
public:

  /**
    Static ID of the BoolErr record.
  */
  static const unsigned int id;

  enum { ErrorUnknown, 
   ErrorNull, 
   ErrorDivZero, 
   ErrorValue, 
   ErrorRef, 
   ErrorName,
   ErrorNum, 
   ErrorNA };

  /**
   * Creates a new BoolErr record.
   */
  BoolErrRecord();

  /**
   * Destroys the BoolErr record.
   */
  virtual ~BoolErrRecord();

  /**
   * Returns true if the record specifies boolean value.
   */
  bool isBool() const;

  /**
   * Returns true if the record specifies error code.
   */
  bool isError() const;

  /**
   * Returns boolean value of the cell. It is valid only when isBool()
   * returns true.
   */
  bool value() const;

  /**
   * Returns error code of the cell. It is valid only when isError()
   * returns true.
   */
  unsigned errorCode() const;
  
  /**
   * Returns error code of the cell as string. It is valid only when isError()
   * returns true.
   */
  const char* errorCodeAsString() const;

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "BOOLERR"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  BoolErrRecord( const BoolErrRecord& );
  BoolErrRecord& operator=( const BoolErrRecord& );

  class Private;
  Private* d;
};

/**
  Class BottomMarginRecord holds information about bottom margin.
  
 */
class BottomMarginRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new BottomMargin record.
   */
  BottomMarginRecord();
  
  /**
   * Destroy the record.
   */
  ~BottomMarginRecord();
  
  /**
   * Gets the bottom margin (in inches).
   */
  double bottomMargin() const;
  
  /**
   * Sets the new bottom margin (in inches).
   */
  void setBottomMargin( double m );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "BOTTOMMARGIN"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  BottomMarginRecord( const BottomMarginRecord& );
  BottomMarginRecord& operator=( const BottomMarginRecord& );

  class Private;
  Private* d;
};

/**
  Class BoundSheetRecord represents BoundSheet record, which defines a sheet
  within the workbook. There must be exactly one BoundSheet record for 
  each sheet.
  
  BoundSheet record stores information about sheet type, sheet name, and 
  the corresponding BOF record.
  
  \sa BOFRecord
 */
 
// TODO support for strong visible

class BoundSheetRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new BoundSheet record.
   */
  BoundSheetRecord();

  /**
   * Destroys the BoundSheet record.
   */
  virtual ~BoundSheetRecord();
  
  /**
   * Type of the sheet.
   */
  enum { Worksheet=0, Chart=2, VBModule=6 };
  
  /**
   * Sets the type of the BoundSheet. Possible values are 
   * BoundSheet::Worksheet, BoundSheet::Chart and BoundSheet::VBModule.
   */
  void setType( unsigned type );
  
  /**
   * Returns the type of the BoundSheet. Possible values are 
   * BoundSheet::Worksheet, BoundSheet::Chart and BoundSheet::VBModule.
   */
  unsigned type() const;
  
  /**
   * Returns the type of the BoundSheet as string. For example, if 
   * type of BoundSheet is BoundSheet::Chart, then this function returns
   * "Chart".
   */
  const char* typeAsString() const;
  
  /**
   * Sets the visibility of the sheet.
   */
  void setVisible( bool visible );
  
  /**
   * Returns true if the sheet is visible.
   */
  bool visible() const;
  
  /**
   * Sets the name of the sheet.
   */
  void setSheetName( const UString& name );
  
  /**
   * Returns the name of the sheet.
   */
  UString sheetName() const;
  
  /**
   * Sets the position of the BOF record associated with this BoundSheet.
   */
  void setBofPosition( unsigned pos );
  
  /**
   * Returns the position of the BOF record associated with this BoundSheet.
   */
  unsigned bofPosition() const;

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
   \reimpl
   */
  virtual const char* name(){ return "BOUNDSHEET"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  BoundSheetRecord( const BoundSheetRecord& );
  BoundSheetRecord& operator=( const BoundSheetRecord& );

  class Private;
  Private* d;
};

/**
  Class ColInfoRecord represents ColInfo record, which provides information
  (such as column width and formatting) for a span of columns.
 */
class ColInfoRecord : public Record, public ColumnSpanInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new ColInfo record.
   */
  ColInfoRecord();

  /**
   * Destroys the record.
   */
  virtual ~ColInfoRecord();
  
  /**
   * Returns the XF index for the formatting of the column(s).
   *
   * \sa setXfIndex
   */
  unsigned xfIndex() const;
  
  /**
   * Sets the XF index for the formatting of the column(s).
   *
   * \sa xfIndex
   */
  void setXfIndex( unsigned i );
  
  /**
   * Returns the width of the column(s), specified in 1/256 of 
   * a character width. The exact width (in pt or inch) could only be 
   * calculated given the base character width for the column format.
   *
   * \sa setWidth
   */
  unsigned width() const;
  
  /**
   * Sets the width of the column(s), specified in 1/256 of 
   * a character width. The exact width (in pt or inch) could only be 
   * calculated given the base character width for the column format.
   *
   * \sa width
   */
  void setWidth( unsigned w );
  
  /**
   * Returns true if the columns should be hidden, i.e not visible.
   *
   * \sa setHidden
   */
  bool hidden() const;
  
  /**
   * Sets whether columns should be hidden or visible.
   *
   * \sa hidden
   */
  void setHidden( bool h );

  /**
   * Returns true if the columns should be collapsed.
   *
   * \sa setCollapsed
   */
  bool collapsed() const;
  
  /**
   * Sets whether columns should be collapsed or not.
   *
   * \sa collapsed
   */
  void setCollapsed( bool c );
  
  /**
   * Returns the outline level of the columns. If it is 0, then
   * the columns are not outlined.
   *
   * \sa setOutlineLevel
   */
  unsigned outlineLevel() const;
  
  /**
   * Sets the outline level of the columns. If it is 0, then
   * the columns are not outlined.
   *
   * \sa outlineLevel
   */
  void setOutlineLevel( unsigned l );
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "COLINFO"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   ColInfoRecord( const ColInfoRecord& );
   ColInfoRecord& operator=( const ColInfoRecord& );

   class Private;
   Private *d;
};

/**
  Class Date1904Record represents Date1904 record, which specifies
  reference date for displaying date value of given serial number.
 */
class Date1904Record : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new Date1904 record.
   */
  Date1904Record();
  
  /**
   * Destroy the record.
   */
  ~Date1904Record();
  
  /**
   * Returns true if the reference date is 1st of January, 1904 or false
   * if the reference date is 31st of December, 1899.
   * 
   * \sa setRef1904
   */
  bool ref1904() const;
  
  /**
   * If r is true, sets the reference date to 1st of January, 1904. Else,
   * sets the reference date to 31st of December, 1899.
   * 
   * \sa ref1904
   */
  void setRef1904( bool r );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "DATE1904"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  Date1904Record( const Date1904Record& );
  Date1904Record& operator=( const Date1904Record& );

  class Private;
  Private* d;
};

/**
  Class EOFRecord represents Dimension record, which contains the range address 
  of the used area in the current sheet.
 */
class DimensionRecord : public Record
{
public:

  /**
   * Creates a new Dimension record.
   */
  DimensionRecord();
  
  /**
   \reimpl
   */
  virtual const char* name(){ return "DIMENSION"; }
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );
};

/**
  Class EOFRecord represents EOF record, which marks the end of records
  for a specific object. EOF record should be always in pair with BOF Record.

  \sa BOFRecord

 */
class EOFRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new EOF record.
   */
  EOFRecord();

  /**
   * Destroy the record.
   */
  virtual ~EOFRecord();
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "EOF"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   EOFRecord( const EOFRecord& );
   EOFRecord& operator=( const EOFRecord& );
};

/**
  Class FontRecord represents Font record, which has the information
  about specific font used in the document. Several Font records creates
  a font table, whose index will be referred in XFormat records.
  
  A note about weirdness: font index #4 is never used. Hence, the first Font
  record will be index #0, the second is #1, the third is #2, the fourth is
  #3, but the fourth will be index #5.

  \sa XFRecord

 */
class FontRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new Font record.
   */
  FontRecord();
  
  /**
   * Creates a copy of another Font record.
   */
  FontRecord( const FontRecord& fr );
  
  /**
   * Assigns from another Font record.
   */
  FontRecord& operator=( const FontRecord& fr );

  /**
   * Destroy the record.
   */
  virtual ~FontRecord();
  
  enum { 
    Normal = 0, 
    Superscript = 1,
    Subscript = 2 };
    
  enum { 
    None = 0, 
    Single = 1, 
    Double = 2, 
    SingleAccounting = 3, 
    DoubleAccounting = 4 };
  
  unsigned height() const;
  void setHeight( unsigned h );
  
  /**
   * Returns the name of font, e.g "Arial".
   *
   * \sa setFontName
   */
  UString fontName() const;
  
  /**
   * Sets the name of the font.
   *
   * \sa fontName
   */
  void setFontName( const UString& fn );

  // FIXME what is this font family ? index ?  
  unsigned fontFamily() const;
  void setFontFamily( unsigned f );
  
  // FIXME and character set index ?
  unsigned characterSet() const;
  void setCharacterSet( unsigned s );
  
  /**
   * Returns index of the color of the font.
   *
   * \sa setColorIndex
   */
  unsigned colorIndex() const;
  
  /**
   * Sets the index of the color of the font.
   *
   * \sa colorIndex
   */
  void setColorIndex( unsigned c );
  
  /**
   * Returns the boldness of the font. Standard values are 400 for normal
   * and 700 for bold.
   *
   * \sa setBoldness
   */
  unsigned boldness() const;
  
  /**
   * Sets the boldness of the font. Standard values are 400 for normal
   * and 700 for bold.
   *
   * \sa boldness
   */
  void setBoldness( unsigned b );
  
  /**
   * Returns true if italic has been set.
   *
   * \sa setItalic
   */  
  bool italic() const;
  
  /**
   * If i is true, italic is set on; otherwise italic is set off.
   *
   * \sa italic
   */  
  void setItalic( bool i );
  
  /**
   * Returns true if strikeout has been set.
   *
   * \sa setStrikeout
   */
  bool strikeout() const;
  
  /**
   * If s is true, strikeout is set on; otherwise strikeout is set off.
   *
   * \sa strikeout
   */
  void setStrikeout( bool s );
  
  /**
   * Returns Font::Superscript if superscript is set, or Font::Subscript
   * if subscript is set, or Font::Normal in other case.
   *
   * \sa setScript
   */
  unsigned script() const;
  
  /**
   * Sets the superscript or subscript. If s is Font::Superscript, then
   * superscript is set. If s is Font::Subscript, then subscript is set.
   *
   * \sa script
   */
  void setScript( unsigned s );
  
  /**
   * Returns the underline style of the font. Possible values are
   * Font::None, Font::Single, Font::Double, Font::SingleAccounting and
   * Font::DoubleAccounting.
   *
   * \sa setUnderline
   */  
  unsigned underline() const;
  
  /**
   * Sets the underline style of the font. Possible values are
   * Font::None, Font::Single, Font::Double, Font::SingleAccounting and
   * Font::DoubleAccounting.
   *
   * \sa underline
   */  
  void setUnderline( unsigned u );
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "FONT"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   class Private;
   Private *d;
};

/**
  Class FormatRecord contains information about a number format. 
  All Format records occur together in a sequential list.
  An XFRecord might refer to the specific Format record using
  an index to that list.

  \sa XFRecord

 */
class FormatRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new Format record.
   */
  FormatRecord();
  
  /**
   * Destroys the Format record.
   */
  ~FormatRecord();
  
  /**
   * Creates a copy of Format record.
   */
  FormatRecord( const FormatRecord& fr );
  
  /**
   * Assigns from another Format record.
   */
  FormatRecord& operator=( const FormatRecord& fr );
  
  /**
   * Returns the index of the format. Each format specified by Format record
   * has unique index which will be referred by XF Record.
   *
   * \sa setIndex
   */
  unsigned index() const;
  
  /**
   * Sets the index of the format. Each format specified by Format record
   * has unique index which will be referred by XF Record.
   *
   * \sa index
   */
  void setIndex( unsigned i );

  /**
   * Returns the formatting string of the format, e.g "0.00" for 2 decimal
   * places number formatting.
   *
   * \sa setFormatString
   */
  UString formatString() const;
  
  /**
   * Sets the formatting string of the format.
   *
   * \sa formatString
   */
  void setFormatString( const UString& fs );
    
  /**
   \reimpl
   */
  virtual const char* name(){ return "FORMAT"; }
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

private:
   class Private;
   Private *d;
};

/**
  Class LabelRecord represents a cell that contains a string.
   
  In Excel 97 and later version, it is replaced by LabelSSTRecord. However,
  Excel 97 can still load LabelRecord.

  \sa LabelSSTRecord

 */
class LabelRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new Label record.
   */
  LabelRecord();

  /**
   * Destroys the record.
   */
  virtual ~LabelRecord();

  /**
   * Returns the label string.
   */
  UString label() const;
  
  /**
   * Sets the label string.
   */
  void setLabel( const UString& l );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "LABEL"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   LabelRecord( const LabelRecord& );
   LabelRecord& operator=( const LabelRecord& );

   class Private;
   Private *d;
};

/**
  Class LabelSSTRecord represents a cell that contains a string. The actual
  string is store in a global SST (Shared String Table), see SSTRecord for
  details. This record only provide an index, which should be used to get
  the string in the corresponding SST.

  \sa SSTRecord

 */
class LabelSSTRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new LabelSST record.
   */
  LabelSSTRecord();

  /**
   * Destroys the record.
   */
  virtual ~LabelSSTRecord();

  /**
   * Returns the SST index. This is the index to the global SST which hold
   * every label strings used in SST record. 
   */
  unsigned sstIndex() const;

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "LABELSST"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   LabelSSTRecord( const LabelSSTRecord& );
   LabelSSTRecord& operator=( const LabelSSTRecord& );

   class Private;
   Private *d;
};

/**
  Class LeftMarginRecord holds information about left margin.
  
 */
class LeftMarginRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new LeftMargin record.
   */
  LeftMarginRecord();
  
  /**
   * Destroy the record.
   */
  ~LeftMarginRecord();
  
  /**
   * Gets the left margin (in inches).
   */
  double leftMargin() const;
  
  /**
   * Sets the new left margin (in inches).
   */
  void setLeftMargin( double m );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "LEFTMARGIN"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  LeftMarginRecord( const LeftMarginRecord& );
  LeftMarginRecord& operator=( const LeftMarginRecord& );

  class Private;
  Private* d;
};



/**
  Class MulBlankRecord represents a cell range containing blank cells. 
  All cells are located in the same row.
  
  \sa BlankRecord
 */
 
class MulBlankRecord : public Record, public CellInfo, public ColumnSpanInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new MulBlank record.
   */
  MulBlankRecord();

  /**
   * Destroys the record.
   */
  virtual ~MulBlankRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   * Returns XF index of ith column.
   */
  unsigned xfIndex( unsigned i ) const;

  /**
   \reimpl
   */
  virtual const char* name(){ return "MULBLANK"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   MulBlankRecord( const MulBlankRecord& );
   MulBlankRecord& operator=( const MulBlankRecord& );

   class Private;
   Private *d;

   // from CellInfo, we don't need it
   // mark as private so nobody can call them
   virtual unsigned column() const { return CellInfo::column(); }
   virtual unsigned xfIndex() const { return CellInfo::xfIndex(); }
};

/**
  Class MulRKRecord represents a cell range containing RK value cells. 
  These cells are located in the same row.
  
  \sa RKRecord
 */

class MulRKRecord : public Record, public CellInfo, public ColumnSpanInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new MulRK record.
   */
  MulRKRecord();

  /**
   * Destroys the record.
   */
  virtual ~MulRKRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   Returns XF index of ith column.
   */
  unsigned xfIndex( unsigned i ) const;
  
  /**
   * Returns true if the record holds an integer value.
   *
   * \sa asInteger
   */
  bool isInteger( unsigned i ) const;
  
  /**
   * Returns the integer value specified by the record. It is only valid
   * when isInteger returns true.
   *
   * \sa isInteger, asFloat
   */
  int asInteger( unsigned i ) const;
  
  /**
   * Returns the floating-point value specified by the record. It is only valid
   * when isInteger returns false.
   *
   * \sa asInteger
   */
  double asFloat( unsigned i ) const;
  
  /**
   \reimpl
   */
  virtual const char* name(){ return "MULRK"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   MulRKRecord( const MulRKRecord& );
   MulRKRecord& operator=( const MulRKRecord& );

   class Private;
   Private *d;

   // from CellInfo, we don't need it
   // mark as private so nobody can call them
   virtual unsigned column() const { return CellInfo::column(); }
   virtual unsigned xfIndex() const { return CellInfo::xfIndex(); }
};

/**
  Class NumberRecord represents a cell that contains a floating point value. 

 */
class NumberRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new Number record.
   */
  NumberRecord();

  /**
   * Destroys the record.
   */
  virtual ~NumberRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
   * Returns the floating-point value specified by the record.
   *
   * \sa setNumber
   */
  double number() const;
  
  /**
   * Sets the floating-point value specified by the record.
   *
   * \sa number
   */
  void setNumber( double f );

  /**
   \reimpl
   */
  virtual const char* name(){ return "NUMBER"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   NumberRecord( const NumberRecord& );
   NumberRecord& operator=( const NumberRecord& );

   class Private;
   Private *d;
};

/**
  Class PaletteRecord lists colors.
  
 */
class PaletteRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new Palette record.
   */
  PaletteRecord();
  
  /**
   * Destroy the record.
   */
  ~PaletteRecord();

  /**
   * Gets the n-th color.
   */
  Color color( unsigned i ) const;

  /**
   * Returns the number of colors in the palette.
   */
  unsigned count() const;
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "PALETTE"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  PaletteRecord( const PaletteRecord& );
  PaletteRecord& operator=( const PaletteRecord& );

  class Private;
  Private* d;
};



/**
  Class RightMarginRecord holds information about right margin.
  
 */
class RightMarginRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new RightMargin record.
   */
  RightMarginRecord();
  
  /**
   * Destroy the record.
   */
  ~RightMarginRecord();
  
  /**
   * Gets the right margin (in inches).
   */
  double rightMargin() const;
  
  /**
   * Sets the new right margin (in inches).
   */
  void setRightMargin( double m );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "RIGHTMARGIN"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  RightMarginRecord( const RightMarginRecord& );
  RightMarginRecord& operator=( const RightMarginRecord& );

  class Private;
  Private* d;
};



/**
  Class RKRecord represents a cell that contains an RK value, 
  i.e encoded integer or floating-point value.

 */
class RKRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new RK record.
   */
  RKRecord();

  /**
   * Destroys the record.
   */
  virtual ~RKRecord();

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
   * Returns true if the record holds an integer value.
   *
   * \sa asInteger, isFloat
   */
  bool isInteger() const;
  
  /**
   * Returns true if the record holds a floating-point value.
   *
   * \sa asFloat, isInteger
   */
  bool isFloat() const;
    
  /**
   * Returns the integer value specified by the record. It is only valid
   * when isInteger returns true.
   *
   * \sa isInteger, asFloat
   */
  int asInteger() const;
  
  /**
   * Returns the floating-point value specified by the record. It is only valid
   * when isFloat returns true.
   *
   * \sa isFloat, asInteger
   */
  double asFloat() const;
  
  /**
   * Sets the integer value to be specified by the record. 
   *
   * \sa setFloat
   */
  void setInteger( int i );
  
  /**
   * Sets the floating-point value to be specified by the record. 
   *
   * \sa setFloat
   */
  void setFloat( double f );

  /**
   \reimpl
   */
  virtual const char* name(){ return "RK"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   RKRecord( const RKRecord& );
   RKRecord& operator=( const RKRecord& );

   class Private;
   Private *d;
};

/**
  Class RStringRecord represents a cell that contains rich-text string.
   
  In Excel 97 and later version, it is replaced by LabelSSTRecord. However,
  Excel 97 is still able to load RStringRecord.

  \sa LabelRecord
  \sa LabelSSTRecord

 */
class RStringRecord : public Record, public CellInfo
{
public:

  static const unsigned int id;

  /**
   * Creates a new Label record.
   */
  RStringRecord();

  /**
   * Destroys the record.
   */
  virtual ~RStringRecord();

  /**
   * Returns the label string.
   *
   * \sa setLabel
   */
  UString label() const;
  
  /**
   * Sets the label string.
   *
   * \sa label
   */
  void setLabel( const UString& l );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "RSTRING"; }
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
   // no copy or assign
   RStringRecord( const RStringRecord& );
   RStringRecord& operator=( const RStringRecord& );

   class Private;
   Private *d;
};


/**
  Class SSTRecord represents SST record, which holds the shared string
  table of the workbook.

  \sa LabelSSTRecord
  
 */
class SSTRecord : public Record
{
public:

  static const unsigned int id;
  
  /**
   * Creates a new SST record.
   */ 
  SSTRecord();
  
  /**
   * Destroys the record.
   */ 
  virtual ~SSTRecord();

  /**
   \reimpl
   */  
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
    Returns the number of available string in this string table.
   */  
  unsigned count() const;
  
  /**
    Returns the string at specified index. 
    Note that index must be less than count().
    If index is not valid, this will return UString::null.    
   */  
  UString stringAt( unsigned index ) const;
  
  /**
   \reimpl
   */  
  virtual const char* name(){ return "SST"; }
  
  /**
   \reimpl
   */  
  virtual void dump( std::ostream& out ) const;
  
private:
   // no copy or assign
   SSTRecord( const SSTRecord& );
   SSTRecord& operator=( const SSTRecord& );

   class Private;
   Private *d;
};


/**
  Class TopMarginRecord holds information about top margin.
  
 */
class TopMarginRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new TopMargin record.
   */
  TopMarginRecord();
  
  /**
   * Destroy the record.
   */
  ~TopMarginRecord();
  
  /**
   * Gets the top margin (in inches).
   */
  double topMargin() const;
  
  /**
   * Sets the new top margin (in inches).
   */
  void setTopMargin( double m );

  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );

  /**
   \reimpl
   */
  virtual const char* name(){ return "TOPMARGIN"; }

  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  // no copy or assign
  TopMarginRecord( const TopMarginRecord& );
  TopMarginRecord& operator=( const TopMarginRecord& );

  class Private;
  Private* d;
};

#if 0
  
  unsigned formatIndex;   // see FORMAT record
  bool cellLocked;
  bool formulaHidden;
  bool styleFormat;
  unsigned parentIndex;
  unsigned verticalAlignment;
  bool textWrapped;
  unsigned rotationAngle;
  unsigned indentLevel;
  bool shrinkContent;
  unsigned usedAttrib;
  unsigned leftBorder;
  unsigned rightBorder;
  unsigned topBorder;
  unsigned bottomBorder;
  unsigned leftColorIndex;
  unsigned rightColorIndex;
  unsigned topColorIndex;
  unsigned bottomColorIndex;
  bool diagonalFromTopLeft;
  bool diagonalFromRightLeft;
  unsigned diagonalColorIndex;
  unsigned diagonalLineStyle;
  unsigned fillPattern;
  unsigned patternForeColorIndex;
  unsigned patternBackColorIndex;
#endif
  
/**
  Class XFRecord holds information of XF (eXtended Format) which specifies
  many kind of properties of a specific cell. It will be referred 
  by record derived from CellInfo, in the member function xfIndex().
        
 */
class XFRecord : public Record
{
public:

  static const unsigned int id;

  /**
   * Creates a new XF record.
   */
  XFRecord();
  
  /**
   * Creates a copy of XF record.
   */
  XFRecord( const XFRecord& xf );
  
  /**
   * Assigns from another XF record.
   */
  XFRecord& operator=( const XFRecord& xf );
  
  /**
   * Destroy the record.
   */
  ~XFRecord();
  
  /**
   * Gets the index of the font for use in this XFormat. The index
   * refers to the font table.
   *
   * \sa setFontIndex, FontRecord
   */
  unsigned fontIndex() const;
  
  /**
   * Sets the index of the font for use in this XFormat. The index
   * refers to the font table.
   *
   * \sa fontIndex, FontRecord
   */
  void setFontIndex( unsigned fi );

  /**
   * Gets the index of the number format for use in this XFormat. The index
   * refers to the format table.
   *
   * \sa setFormatIndex, FormatRecord
   */
  unsigned formatIndex() const;
  
  /**
   * Sets the index of the number format for use in this XFormat. The index
   * refers to the format table.
   *
   * \sa formatIndex, FormatRecord
   */
  void setFormatIndex( unsigned fi );
  
  /**
   * Returns true if the cells using this format should be locked.
   *
   * \sa setLocked
   */  
  bool locked() const;
  
  /**
   * Sets whether the cells using this format should be locked or not.
   *
   * \sa locked
   */  
  void setLocked( bool l );
  
  /**
   * Returns true if the formula of the cells using this format 
   * should be hidden from user.
   *
   * \sa setFormulaHidden
   */  
  bool formulaHidden() const;

  /**
   * Sets whether the formula of the cells using this format 
   * should be hidden or should be visible.
   *
   * \sa formulaHidden
   */  
  void setFormulaHidden( bool f );
  
  /**
   * Returns the index of the parent stlye of this format.
   * This refers to the index of the XFormat table which is constructed 
   * from a series of XFormat records.
   *
   * \sa setParentStyle
   */  
  unsigned parentStyle() const;
  
  /**
   * Sets the index of the parent stlye of this format.
   * This refers to the index of the XFormat table which is constructed 
   * from a series of XFormat records.
   *
   * \sa parentStyle
   */  
  void setParentStyle( unsigned ps );

  enum { 
    General = 0, 
    Left, 
    Centered, 
    Right, 
    Filled, 
    Justified,
    CenteredSelection,
    Distributed };
  
  /**
   * Gets the horizontal alignment, e.g Left.
   */
  unsigned horizontalAlignment() const;
  
  /**
   * Sets the horizontal alignment, e.g Left.
   */
  void setHorizontalAlignment( unsigned ha );
  
  /**
   * Returns human-readable string representation of the horizontal alignment.
     For example, XFRecord::Left will return "Left".
   */
  const char* horizontalAlignmentAsString() const; 
  
  enum { 
    Top = 0, 
    VCentered = 1, 
    Bottom = 2, 
    VJustified = 3, 
    VDistributed = 4 };
  
  /**
   * Gets the vertical alignment, e.g Bottom.
   *
   * \sa setVerticalAlignment
   */
  unsigned verticalAlignment() const;
  
  /**
   * Sets the vertical alignment, e.g Top.
   *
   * \sa verticalAlignment
   */
  void setVerticalAlignment( unsigned va );
  
  /**
   * Returns human-readable string representation of the vertical alignment.
     For example, XFRecord::Top will return "Top".
   */
  const char* verticalAlignmentAsString() const; 
  
  /**
   \reimpl
   */
  virtual const char* name(){ return "XF"; }
  
  /**
   \reimpl
   */
  virtual void setData( unsigned size, const unsigned char* data );
  
  /**
   \reimpl
   */
  virtual void dump( std::ostream& out ) const;

private:
  class Private;
  Private* d;
};


class ExcelReader: public Reader
{
public:
  ExcelReader();
  virtual ~ExcelReader();
  
  virtual Workbook* load( const char* filename );
    
private:

  void handleRecord( Record* record );
    
  void handleBoundSheet( BoundSheetRecord* record );
  void handleBOF( BOFRecord* record );
  void handleBoolErr( BoolErrRecord* record );
  void handleBlank( BlankRecord* record );
  void handleColInfo( ColInfoRecord* record );
  void handleDate1904( Date1904Record* record );
  void handleFormat( FormatRecord* record );
  void handleFont( FontRecord* record );
  void handleLabel( LabelRecord* record );
  void handleLabelSST( LabelSSTRecord* record );
  void handleMulBlank( MulBlankRecord* record );
  void handleMulRK( MulRKRecord* record );
  void handleNumber( NumberRecord* record );
  void handleRString( RStringRecord* record );
  void handleRK( RKRecord* record );
  void handleSST( SSTRecord* record );
  void handleXF( XFRecord* record );    
  
  Format convertFormat( const XFRecord& xf );
  Format convertFormat( unsigned xfIndex );
  
  // no copy or assign
  ExcelReader( const ExcelReader& );
  ExcelReader& operator=( const ExcelReader& );
  
  class Private;
  Private* d;
};


}; // namespace Sidewinder


#endif // SIDEWINDER_EXCEL_H
