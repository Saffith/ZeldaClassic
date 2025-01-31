//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "sprite.h"
#include "zelda.h"
#include "maps.h"
#include "tiles.h"

/*
void sprite::check_conveyor()
{
  if (conveyclk<=0)
  {
    int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
    if((ctype>=cOLD_CVUP) && (ctype<=cOLD_CVRIGHT))
    {
      switch (ctype-cOLD_CVUP)
      {
        case up:
        if(!_walkflag(x,y+8-2,2))
        {
          y=y-2;
        }
        break;
        case down:
        if(!_walkflag(x,y+15+2,2))
        {
          y=y+2;
        }
        break;
        case left:
        if(!_walkflag(x-2,y+8,1))
        {
          x=x-2;
        }
        break;
        case right:
        if(!_walkflag(x+15+2,y+8,1))
        {
          x=x+2;
        }
        break;
      }
    }
  }
}
*/

void sprite::handle_sprlighting()
{
	if(!glowRad) return;
	switch(glowShape)
	{
		case 0:
			doDarkroomCircle(x.getInt()+(hxsz/2), y.getInt()+(hysz/2), glowRad);
			break;
		case 1:
			doDarkroomCone(x.getInt()+(hxsz/2), y.getInt()+(hysz/2), glowRad, NORMAL_DIR(dir));
			break;
	}
}

void sprite::check_conveyor()
{
    int deltax=0;
    int deltay=0;
    
    if(conveyclk<=0 && (z==0 || (tmpscr->flags2&fAIRCOMBOS)))
    {
        int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
        deltax=combo_class_buf[ctype].conveyor_x_speed;
        deltay=combo_class_buf[ctype].conveyor_y_speed;
        
        if(deltax!=0||deltay!=0)
        {
            if(deltay<0&&!_walkflag(x,y+8-2,2))
            {
                y=y-abs(deltay);
            }
            else if(deltay>0&&!_walkflag(x,y+15+2,2))
            {
                y=y+abs(deltay);
            }
            
            if(deltax<0&&!_walkflag(x-2,y+8,1))
            {
                x=x-abs(deltax);
            }
            else if(deltax>0&&!_walkflag(x+15+2,y+8,1))
            {
                x=x+abs(deltax);
            }
        }
    }
}

void movingblock::push(zfix bx,zfix by,int d2,int f)
{
    trigger=false;
    endx=x=bx;
    endy=y=by;
    dir=d2;
    oldflag=f;
    word *di = &(tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]);
    byte *ci = &(tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]);
    //   bcombo = ((*di)&0xFF)+(tmpscr->cpage<<8);
    bcombo =  tmpscr->data[(int(y)&0xF0)+(int(x)>>4)];
    oldcset = tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)];
    cs     = (isdungeon() && !get_bit(quest_rules, qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
    tile = combobuf[bcombo].tile;
    flip = combobuf[bcombo].flip;
    //   cs = ((*di)&0x700)>>8;
    *di = tmpscr->undercombo;
    *ci = tmpscr->undercset;
    putcombo(scrollbuf,x,y,*di,*ci);
    clk=32;
    blockmoving=true;
}

bool movingblock::animate(int index)
{
    //these are here to bypass compiler warnings about unused arguments
    index=index;
    if(fallclk)
	{
		if(fallclk == PITFALL_FALL_FRAMES)
			sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			blockmoving=false;
		}
		clk = 0;
		return false;
	}
	if(drownclk)
	{
		if(drownclk == WATER_DROWN_FRAMES);
			//sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
			//!TODO: Drown SFX
		if(!--drownclk)
		{
			blockmoving=false;
		}
		clk = 0;
		return false;
	}
    if(clk<=0)
        return false;
        
    move((zfix)0.5);
    
    if(--clk==0)
    {
        bool bhole=false;
        blockmoving=false;
		
		if(fallCombo = getpitfall(x+8,y+8))
		{
			fallclk = PITFALL_FALL_FRAMES;
		}
		/*
		//!TODO: Moving Block Drowning
		if(drownCombo = iswaterex(MAPCOMBO(x+8,y+8), currmap, currscr, -1, x+8,y+8, false, false, true))
		{
			drownclk = WATER_DROWN_FRAMES;
		}
		*/
		
        int f1 = tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)];
        int f2 = MAPCOMBOFLAG(x,y);
        if(!fallclk && !drownclk)
	{
		tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]=bcombo;
		tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]=oldcset;
        }
        if(!fallclk && !drownclk && ((f1==mfBLOCKTRIGGER)||f2==mfBLOCKTRIGGER))
        {
            trigger=true;
            tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
            //the above line used to be in the following if statement.
            //However, it caused inherent-flag pushblocks to not lock into
            //block trigger combos unless the block trigger is also an
            //inherent flag
            /*
            if(f2==mfBLOCKTRIGGER)
            {
              tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
            }
            */
        }
        
        if((f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE)
        {
            tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]+=1;
            bhole=true;
            //tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]=;
        }
        
        if(bhole)
        {
            tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfNONE;
			if(fallclk||drownclk)
			{
				fallclk = 0;
				drownclk = 0;
				return false;
			}
        }
        else if(!fallclk&&!drownclk)
        {
            f2 = MAPCOMBOFLAG(x,y);
            
            if(!((f2==mfPUSHUDINS && dir<=down) ||
                    (f2==mfPUSHLRINS && dir>=left) ||
                    (f2==mfPUSHUINS && dir==up) ||
                    (f2==mfPUSHDINS && dir==down) ||
                    (f2==mfPUSHLINS && dir==left) ||
                    (f2==mfPUSHRINS && dir==right) ||
                    (f2==mfPUSH4INS)))
            {
                tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
            }
        }
		if(fallclk||drownclk) return false;
        
        if(oldflag>=mfPUSHUDINS&&oldflag&&!trigger&&!bhole)
        {
            tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=oldflag;
        }
        
        for(int i=0; i<176; i++)
        {
            if(tmpscr->sflag[i]==mfBLOCKTRIGGER||combobuf[tmpscr->data[i]].flag==mfBLOCKTRIGGER)
            {
                trigger=false;
            }
        }
        
        //triggers a secret
        f2 = MAPCOMBOFLAG(x,y);
        
        if((oldflag==mfPUSH4 ||
            (oldflag==mfPUSHUD && dir<=down) ||
            (oldflag==mfPUSHLR && dir>=left) ||
            (oldflag==mfPUSHU && dir==up) ||
            (oldflag==mfPUSHD && dir==down) ||
            (oldflag==mfPUSHL && dir==left) ||
            (oldflag==mfPUSHR && dir==right) ||
            f2==mfPUSH4 ||
            (f2==mfPUSHUD && dir<=down) ||
            (f2==mfPUSHLR && dir>=left) ||
            (f2==mfPUSHU && dir==up) ||
            (f2==mfPUSHD && dir==down) ||
            (f2==mfPUSHL && dir==left) ||
            (f2==mfPUSHR && dir==right)) ||
           trigger)
            //if(oldflag<mfPUSHUDNS||trigger)
        {
            if(hiddenstair(0,true))
            {
                sfx(tmpscr->secretsfx);
            }
            else
            {
                hidden_entrance(0,true,true);
                
                if((combobuf[bcombo].type == cPUSH_WAIT) ||
                        (combobuf[bcombo].type == cPUSH_HW) ||
                        (combobuf[bcombo].type == cPUSH_HW2) || trigger)
                {
                    sfx(tmpscr->secretsfx);
                }
            }
            
            if(isdungeon() && tmpscr->flags&fSHUTTERS)
            {
                opendoors=8;
            }
            
            if(canPermSecret())
            {
                if(get_bit(quest_rules, qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
					(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
                        || combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
                {
                    if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
                }
            }
        }
        
        putcombo(scrollbuf,x,y,bcombo,cs);
    }
    
    return false;
}

/*** end of sprite.cc ***/

