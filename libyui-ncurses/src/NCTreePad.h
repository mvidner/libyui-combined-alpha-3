/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       NCTreePad.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCTreePad_h
#define NCTreePad_h

#include <iosfwd>
#include <vector>

#include "NCTableItem.h"
#include "NCPad.h"
#include "NCstring.h"

class NCTableLine;
class NCTableCol;

using std::vector;


class NCTreePad : public NCPad
{
private:

    friend std::ostream & operator<<( std::ostream & STREAM, const NCTreePad & OBJ );

    NCTreePad & operator=( const NCTreePad & );
    NCTreePad( const NCTreePad & );


    NCursesPad	Headpad;
    bool	dirtyHead;
    bool	dirtyFormat;

    NCTableStyle	 ItemStyle;
    NCTableLine		 Headline;
    vector<NCTableLine*> Items;
    vector<NCTableLine*> visItems;
    wpos		 citem;

    void assertLine( unsigned idx );

protected:

    void	 DirtyFormat() { dirty = dirtyFormat = true; }

    virtual wsze UpdateFormat();

    virtual int  dirtyPad() { return setpos( CurPos() ); }

    virtual int  setpos( const wpos & newpos );
    virtual int  DoRedraw();
    virtual void updateScrollHint();

public:

    NCTreePad( int lines, int cols, const NCWidget & p );
    virtual ~NCTreePad();

public:

    NCursesWindow * Destwin() { return NCPad::Destwin(); }

    virtual void Destwin( NCursesWindow * dwin );

    virtual void wRecoded();

    virtual wpos CurPos() const;
    virtual bool handleInput( wint_t key );

public:

    bool SetHeadline( const vector<NCstring> & head );
    
    virtual void SendHead()
    {
	SetHead( Headpad, srect.Pos.C );
	dirtyHead = false;
    }

    unsigned Cols()	const { return ItemStyle.Cols(); }

    unsigned Lines()	const { return Items.size(); }

    unsigned visLines() const { return visItems.size(); }

    void     SetLines( unsigned idx );
    void     SetLines( vector<NCTableLine*> & nItems );
    void     ClearTable()  { SetLines( 0 ); }

    void Append( NCTableLine * item )		{ AddLine( Lines(), item ); }

    void Append( vector<NCTableCol*> & nItems ) { AddLine( Lines(), new NCTableLine( nItems ) ); }

    void AddLine( unsigned idx, NCTableLine * item );
    void DelLine( unsigned idx );

    const NCTableLine * GetCurrentLine() const ;
    const NCTableLine * GetLine( unsigned idx ) const;

    NCTableLine *	ModifyLine( unsigned idx );

    void ShowItem( const NCTableLine * item );
};


#endif // NCTreePad_h