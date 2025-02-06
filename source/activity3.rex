<script language="REXX">
<!--BeginREXX Parse upper arg FormName Start End MaxMotion More
/* DirectOnly == 0=All, 1=Direct 2=Digi/Igate 3=Dots 4=SmallDots */

	if MaxMotion = '' then MaxMotion = 1

	Call RxGetTime('TimeStarted')	/* Need NOW */

	if Start = '' | Start = '0' then
	do
		if RxStatusMessage('RATES;APRS;APRS_QUERY_TRAFFIC_RATES;BucketSize==60;*=') = 0 then
		do
			Call RxSortMessage('RATES','Rates','{StartTime}')
			Start = RxGetIndexValue('RATES','Rates',1,'StartTime')
			Say 'Actually Starting 'Start' With 'RxGetValue('RATES.Total')' Packets in DB'
		end
		else Start = 1
	end

	FontHeight = GdFontGetHeight('GDFONTGIANT')
	FontWidth = GdFontGetWidth('GDFONTGIANT')
	TopMargin = FontHeight / 2 + 2
	BottomMargin = FontHeight / 2 + 2
	LeftMargin = 20/2+2	/* Room for a large circle */
	RightMargin = 9*FontWidth + FORMAT(FontWidth/2,,0) + 2	/* 9 character callsign */
	if TopMargin < LeftMargin then TopMargin = LeftMargin
	if BottomMargin < LeftMargin then BottomMargin = LeftMargin

	ImageCount = 0

	Do While More \= ''

		parse var More RenderStyles iName iMinLat iMinLon iMaxLat iMaxLon iZoom More

		if iZoom \= '' & iZoom \= '0' then
		do
			if POS('X',iZoom) > 1 then
			do
				X = POS('X',iZoom)
				MaxX = left(iZoom,X-1)
				MaxY = substr(iZoom,X+1)
Say 'Max dimensions 'MaxX' x 'MaxY
				LastTileCount = 0
				Do Tiles=400 to 10 By -1
					if RxStatusMessage('COORDS;APRS;APRS_CALC_OSM_TILES;MaxTiles=|Tiles|;Min.Latitude=|iMinLat|;Min.Longitude=|iMinLon|;Max.Latitude=|iMaxLat|;Max.Longitude=|iMaxLon|;LeftMargin=|LeftMargin|;RightMargin=|RightMargin|;TopMargin=|TopMargin|;BottomMargin=|BottomMargin|;*=') = 0 then
					do
						if RxGetValue('COORDS.TileCount') = LastTileCount then iterate
						LastTileCount = COORDS.TileCount
						dx = RxGetValue('COORDS.EndX') - RxGetValue('COORDS.StartX') + 1
						dy = RxGetValue('COORDS.EndY') - RxGetValue('COORDS.StartY') + 1
						if dx*256 <= MaxX & dy*256 <= MaxY then
						do
Say 'Found at '||dx*256||' x '||dy*256||' iZoom 'RxGetValue('COORDS.Zoom')' covering 'RxGetValue('COORDS.TileCount')' Tiles'
							Call PrepareImages(RenderStyles)
							leave
						end
else Say 'NOT at '||dx*256||' x '||dy*256||' iZoom 'RxGetValue('COORDS.Zoom')' covering 'RxGetValue('COORDS.TileCount')' Tiles'
					End
				end
Say 'Continuing with 'ImageCount' images'
			end
			else if RxStatusMessage('COORDS;APRS;APRS_CALC_OSM_TILES;Min.Latitude=|iMinLat|;Min.Longitude=|iMinLon|;Max.Latitude=|iMaxLat|;Max.Longitude=|iMaxLon|;Zoom=|iZoom|;LeftMargin=|LeftMargin|;RightMargin=|RightMargin|;TopMargin=|TopMargin|;BottomMargin=|BottomMargin|;*=') = 0 then
			do
				Call PrepareImages(RenderStyles)
			End
		end
		else if iMaxLon \= '' then
		do
			LastTileCount = 0
			Do Tiles=10 to 400
				if RxStatusMessage('COORDS;APRS;APRS_CALC_OSM_TILES;MaxTiles=|Tiles|;Min.Latitude=|iMinLat|;Min.Longitude=|iMinLon|;Max.Latitude=|iMaxLat|;Max.Longitude=|iMaxLon|;LeftMargin=|LeftMargin|;RightMargin=|RightMargin|;TopMargin=|TopMargin|;BottomMargin=|BottomMargin|;*=') = 0 then
				do
					if RxGetValue('COORDS.TileCount') = LastTileCount then iterate
					LastTileCount = COORDS.TileCount
					Call PrepareImages(RenderStyles)
				End
			end
		end
		else
		do
			Say 'QueryUniqueIGates Does Not Have Default Coordinates and Cannot zoom!'

/*			LastTileCount = 0
			Do Tiles=10 to 400
				if RxStatusMessage('COORDS;APRS;APRS_CALC_OSM_TILES;MaxTiles=|Tiles|;Min.Latitude=|PATHS.Min.Latitude|;Min.Longitude=|PATHS.Min.Longitude|;Max.Latitude=|PATHS.Max.Latitude|;Max.Longitude=|PATHS.Max.Longitude|;LeftMargin=|LeftMargin|;RightMargin=|RightMargin|;TopMargin=|TopMargin|;BottomMargin=|BottomMargin|;*=') = 0 then
				do
					if RxGetValue('COORDS.TileCount') = LastTileCount then iterate
					LastTileCount = COORDS.TileCount
					Call PrepareImages(RenderStyles)
				End
			End
*/
		End
	end
	
	Call RxGetTime('TimeImages')
	Say 'Preparing 'ImageCount' Images took '||TimeString(TimeImages-TimeStarted)||', Querying IGates'
	if ImageCount = 0 then return 0

	if SYMBOL('NeverDefined') = 'VAR' then
	do
		if RxSendMessage('IGATES;APRS;APRS_QUERY_UNIQUE_IGATES;OnlyKnown==1;StartTime=|Start|;EndTime=|End|;MaxMotion=|MaxMotion|;ReallyAll==1;*=') \= 0 then
		do
			Say 'Failed to Query IGates, Reason 'RX.INTERRM
			return
		end
		IGateCount = RxGetValue('IGATES.Count')
		Say 'Mapping 'IGateCount' IGates'
		if IGateCount = 0 then return 0
		Call RxSortMessage('IGATES','Stations','{!PacketsGated}')	/* Put the busiest stations first */
	end
	else
	do
		if RxSendMessage('IGATES;APRS;APRS_QUERY_STATIONS;HeardSince=|Start|;ReallyAll==1;*=') \= 0 then
		do
			Say 'Failed to Query Movers, Reason 'RX.INTERRM
			return
		end
		IGateCount = RxGetValue('IGATES.Count')
		Say 'Mapping 'IGateCount' Movers across 'IGateCount' IGates'
		if IGateCount = 0 then return 0
		Call RxSortMessage('IGATES','Stations','{!PacketsReceived}')	/* Put the oldest/active stations first */
	end

	Call GenerateImages()

	Do Image = 1 to ImageCount
		Say Image'/'ImageCount':Output is 'Images.Image.$Output
	End

	if SYMBOL('DeltaTime') = 'VAR' & SYMBOL('TimeRemaining') = 'VAR' then
		Say 'Total Elapsed Time '||TimeString(DeltaTime+TimeRemaining)

Return

PrepareImages:
parse arg RenderStyles
	do gii = 1 to LENGTH(RenderStyles)
		Call PrepareImage(SUBSTR(RenderStyles,gii,1))
	end
return

PrepareImage:
Parse Arg RenderStyle

	Image = ImageCount + 1
	ImageCount = Image

	Images.Image.$iName = iName
	Images.Image.$RenderStyle = RenderStyle

	if RenderStyle = 0 then
	do
		Images.Image.$Title = 'All Traffic'
		Images.Image.$DirectOnly = 0
		Images.Image.$Infrastructure = 0
		Images.Image.$MoverDots = 0
		Images.Image.$LabelRelays = 1
		Images.Image.$BigDotRelays = 1
		Images.Image.$SmallDotRelays = 0
		Images.Image.$LabelGates = 1
		Images.Image.$BigDotGates = 1
		Images.Image.$SmallDotGates = 0
	end
	else if RenderStyle = 1 then
	do
		Images.Image.$Title = "Direct IGate Traffic"
		Images.Image.$DirectOnly = 1
		Images.Image.$Infrastructure = 0
		Images.Image.$MoverDots = 0
		Images.Image.$LabelRelays = 0
		Images.Image.$BigDotRelays = 0
		Images.Image.$SmallDotRelays = 0
		Images.Image.$LabelGates = 1
		Images.Image.$BigDotGates = 1
		Images.Image.$SmallDotGates = 0
	end
	else if Images.Image.$RenderStyle = 2 then
	do
		Images.Image.$Title = "Digi/IGate Traffic"
		Images.Image.$DirectOnly = 0
		Images.Image.$Infrastructure = 1
		Images.Image.$MoverDots = 0
		Images.Image.$LabelRelays = 1
		Images.Image.$BigDotRelays = 1
		Images.Image.$SmallDotRelays = 0
		Images.Image.$LabelGates = 1
		Images.Image.$BigDotGates = 1
		Images.Image.$SmallDotGates = 0
	end
	else if Images.Image.$RenderStyle = 3 then
	do
		Images.Image.$Title = "Mover, Digi, IGate"
		Images.Image.$DirectOnly = 0
		Images.Image.$Infrastructure = 0
		Images.Image.$MoverDots = 1
		Images.Image.$LabelRelays = 0
		Images.Image.$BigDotRelays = 1
		Images.Image.$SmallDotRelays = 0
		Images.Image.$LabelGates = 0
		Images.Image.$BigDotGates = 1
		Images.Image.$SmallDotGates = 0
	end
	else
	do
		Images.Image.$RenderStyle = 4
		Images.Image.$Title = "Mover, Digi, IGate"
		Images.Image.$DirectOnly = 0
		Images.Image.$Infrastructure = 0
		Images.Image.$MoverDots = 1
		Images.Image.$LabelRelays = 0
		Images.Image.$BigDotRelays = 0
		Images.Image.$SmallDotRelays = 1
		Images.Image.$LabelGates = 0
		Images.Image.$BigDotGates = 0
		Images.Image.$SmallDotGates = 1
	end

	Images.Image.$MinLat = RxGetValue('COORDS.Min.Latitude')
	Images.Image.$MaxLat = RxGetValue('COORDS.Max.Latitude')
	Images.Image.$MinLon = RxGetValue('COORDS.Min.Longitude')
	Images.Image.$MaxLon = RxGetValue('COORDS.Max.Longitude')
	Images.Image.$Zoom = RxGetValue('COORDS.Zoom')
	Images.Image.$TotalTiles = RxGetValue('COORDS.TileCount')
	Images.Image.$StartX = RxGetValue('COORDS.StartX')
	Images.Image.$EndX = RxGetValue('COORDS.EndX')
	Images.Image.$StartY = RxGetValue('COORDS.StartY')
	Images.Image.$EndY = RxGetValue('COORDS.EndY')
	Images.Image.$DeltaLon = RxGetValue('COORDS.DeltaLongitude')
	Images.Image.$TileCount = RxGetValue('COORDS.Count')

	Do I = 1 to Images.Image.$TileCount
		Images.Image.$Tiles.I.$Y = RxGetIndexValue('COORDS','Tiles',I,'Y')
		Images.Image.$Tiles.I.$StartLat = RxGetIndexValue('COORDS','Tiles',I,'StartLatitude')
		Images.Image.$Tiles.I.$EndLat = RxGetIndexValue('COORDS','Tiles',I,'EndLatitude')
		Images.Image.$Tiles.I.$DeltaLat = RxGetIndexValue('COORDS','Tiles',I,'DeltaLatitude')
	End

	dx = Images.Image.$EndX - Images.Image.$StartX + 1
	dy = Images.Image.$EndY - Images.Image.$StartY + 1

	TileCount = dx * dy

	Say Image'/'ImageCount':Going for zoom 'Images.Image.$Zoom' with 'TileCount'('Images.Image.$TotalTiles') Tiles'
	Say Image'/'ImageCount':Tiles From 'Images.Image.$StartX'-'Images.Image.$EndX' and 'Images.Image.$StartY'-'Images.Image.$EndY
	Say Image'/'ImageCount':Coords From 'Images.Image.$MinLat' 'Images.Image.$MaxLat' to 'Images.Image.$MinLon' 'Images.Image.$MaxLon

	Images.Image.$img = GdImageCreateTrueColor(dx*256, dy*256)
	if Images.Image.$img = '' then
	do
		Say 'Failed To Create TrueColor Image'
		return
	end

/*
* Get some colors from pallet for drawing
*/
	Images.Image.$white = GdImageColorResolve( Images.Image.$img, 245, 255, 250 )	/* First allocated color is background (white) */
	Images.Image.$black = GdImageColorResolve( Images.Image.$img, 0, 0, 0 )
	Images.Image.$red = GdImageColorResolve(Images.Image.$img, 255,0,0)
	Images.Image.$pink = GdImageColorResolve(Images.Image.$img, 255,128,128)
	Images.Image.$green = GdImageColorResolve(Images.Image.$img, 0,255,0)
	Images.Image.$palegreen = GdImageColorResolve(Images.Image.$img, 128,255,128)

/*
	Tile sources:

	http://tile.openstreetmap.org/5/15/15.png (128.40.168.99 as of 7/2/2009)
	http://tah.openstreetmap.org/Tiles/tile/5/15/15.png
	http://andy.sandbox.cloudmade.com/tiles/cycle/5/15/15.png
	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/1/256/5/15/15.png
	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/2/256/5/15/15.png
	http://tile.cloudmade.com/8bafab36916b5ce6b4395ede3cb9ddea/3/256/5/15/15.png
*/

	z = COORDS.Zoom
	sx = COORDS.StartX
	ex = COORDS.EndX
	sy = COORDS.StartY
	ey = COORDS.EndY

	if SYMBOL('Images.$Got.z.sx.ex.sy.ey') = 'LIT' then
	do
		AllGood = 1
		do X = COORDS.StartX to COORDS.EndX
			Say Image'/'ImageCount':Column '||X||' / '||COORDS.StartX||'-'||COORDS.EndX||' ('||COORDS.EndY-COORDS.StartY+1||' Tiles High)'
			do Y = COORDS.StartY to COORDS.EndY
				/* Say 'Get('||COORDS.Zoom||'/'||X||'/'||Y||'.png)' */
			/*	Body = RxGetFromAuthenticatedAddnl('192.168.10.254:800', 'http://tile.openstreetmap.org/'||COORDS.Zoom||'/'||X||'/'Y||'.png') */
				Body = RxGetFromAuthenticatedAddnl('192.168.10.160:80', '/osm/'||COORDS.Zoom||'/'||X||'/'Y||'.png')
				if RX.INTCODE \= 0 then
				do
					Say 'Get('||COORDS.Zoom||'/'||X||'/'||Y||'.png) Failed With 'RX.INTERRM
					AllGood = 0
				end
				else
				do
					/* Say 'Create('||COORDS.Zoom||'/'||X||'/'||Y||'.png) from 'LENGTH(Body)' Bytes' */
					img = GdImageCreateFromPngPtr(Body)
					if img = '' then
					do
						Say 'CreateImage('||COORDS.Zoom||'/'||X||'/'||Y||'.png) Failed'
						AllGood = 0
					end
					else
					do
						/* Say 'Copy('||COORDS.Zoom||'/'||X||'/'||Y||'.png)' */
						Call GdImageCopy(Images.Image.$img, img, (X-COORDS.StartX)*256,(Y-COORDS.StartY)*256, 0, 0, 256, 256)
						/* Say 'Destroy('||COORDS.Zoom||'/'||X||'/'||Y||'.png)' */
						Call GdImageDestroy(img);
					end
				end
			end
		end
		if AllGood = 1 then Images.$Got.z.sx.ex.sy.ey = Images.Image.$img
	end
	else
	do
		Say Image'/'ImageCount':Copying 'Image.z.sx.ex.sy.ey' Image'
		dx = ex - sx + 1
		dy = ey - sy + 1
		Call GdImageCopy(Images.Image.$img, Images.$Got.z.sx.ex.sy.ey, 0,0, 0, 0, dx*256, dy*256)
	end

/* LineAroundTiles = 1 */
	if SYMBOL('LineAroundTiles') = 'VAR' then
	do X = COORDS.StartX to COORDS.EndX
		do Y = COORDS.StartY to COORDS.EndY
			fx = (X-COORDS.StartX)*256
			fy = (Y-COORDS.StartY)*256
			tx = fx + 255
			ty = fx + 255
			Call GdImageLine(Images.Image.$img,fx,fy,fx,ty,Images.Image.$pink)
			Call GdImageLine(Images.Image.$img,fx,ty,tx,ty,Images.Image.$pink)
			Call GdImageLine(Images.Image.$img,tx,ty,tx,fy,Images.Image.$pink)
			Call GdImageLine(Images.Image.$img,tx,fy,fx,fy,Images.Image.$pink)
		end
	end

	Images.Image.$xSize = GdImageGetWidth(Images.Image.$img)
	Images.Image.$ySize = GdImageGetHeight(Images.Image.$img)
	Say Image'/'ImageCount'('Images.Image.$iName'):Image size:' Images.Image.$xSize 'x' Images.Image.$ySize

	if RenderStyle = 3 then
		Call GdImageSetThickness(Images.Image.$img, 2);
	else	Call GdImageSetThickness(Images.Image.$img, 1);


	Gates.Image.0 = 0
	Relays.Image.0 = 0

return Image

GenerateImages:

	Call RxGetTime('GenerateStarted')
   /*
    * Now draw our lines!
    */
   Do G = 1 to IGateCount
	GateLon = RxGetIndexValue('IGATES','Stations',G,'Last.Longitude')
	GateLat = RxGetIndexValue('IGATES','Stations',G,'Last.Latitude')
	GateID = RxGetIndexValue('IGATES','Stations',G,'StationID')
	Say 'GateID:'GateID
	if GateID = '' then iterate
	if GateID \= RxGetIndexValue('IGATES','Stations',G,'OwnerID') then
	do
		Say 'Skipping Object 'RxGetIndexValue('IGATES','Stations',G,'OwnerID')' 'GateID
		iterate
	end

	Drop Now
	Call RxGetTime('Now')	/* Need NOW */
	SayInterval = 10	/* Speak every 10 seconds */

	if G <= 1 | Now <= GenerateStarted then
	do
		Say End': 'G'/'IGateCount':Query 'GateID' Paths'
		NextSay = Now + SayInterval
	end
	else
	do
		DeltaTime = Now-GenerateStarted
		TimePer = DeltaTime/(G-1)
		TimeRemaining = TimePer*(IGateCount-G+1)
		if Now >= NextSay then
		do
			NextSay = Now + SayInterval
			if SYMBOL('TimeRemaining') = 'VAR' & DATATYPE(TimeRemaining,'N') then
				Say End': 'G'/'IGateCount':Query 'GateID' Paths '||TimeString(TimeRemaining)||' Remaining, ETA '||RxFormatTime('%Y-%m-%d %H:%M:%S',TRUNC(Now+TimeRemaining))||' (Total: '||TimeString(DeltaTime+TimeRemaining)||')'
			else Say End': 'G'/'IGateCount':Query 'GateID' Paths NonNumeric('TimeRemaining')'
		end
	end

	Drop Paths
	Drop Paths.

	if RxStatusMessage(';APRS;APRS_LOOKUP_STATION;StationID=|GateID|;*=') \= 0 then
		Say End': 'G'/'IGateCount':LookupStation('GateID') Failed with 'RX.INTERRM
	else
	/* if RxStatusMessage('PATHS;APRS;APRS_QUERY_UNIQUE_PATHS;IGateID=|GateID|;StartTime=|Start|;EndTime=|End|;*=') = 0 then */
	if RxStatusMessage('PATHS;APRS;APRS_QUERY_UNIQUE_PATHS;OriginID=|GateID|;StartTime=|Start|;EndTime=|End|;*=') = 0 then
	if RxGetValue('PATHS.Count') > 0 then
	Do Image = 1  to ImageCount

		MinLat = RxGetValue('PATHS.Min.Latitude')
		MinLon = RxGetValue('PATHS.Min.Longitude')
		MaxLat = RxGetValue('PATHS.Max.Latitude')
		MaxLon = RxGetValue('PATHS.Max.Longitude')

		if MinLat < Images.Image.$MaxLat & MaxLat > Images.Image.$MinLat then
		if MinLon < Images.Image.$MaxLon & MaxLon > Images.Image.$MinLon then
		do
			PathCount = RxGetValue('PATHS.Count')

			Drop Now
			Call RxGetTime('Now')	/* Need NOW */
			if Now >= NextSay | (SYMBOL('Paths.$ayd') = 'LIT' & PathCount > 200) | PathCount > 1000 then
			do
				Paths.$ayd = 'Done'
				NextSay = Now + SayInterval
				if SYMBOL('TimeRemaining') = 'VAR' & DATATYPE(TimeRemaining,'N') then
					Say End': 'G'/'IGateCount':'Image'/'ImageCount':Processing 'GateID' 'PathCount' Paths '||TimeString(TimeRemaining)||' Remaining, ETA '||RxFormatTime('%Y-%m-%d %H:%M:%S',TRUNC(Now+TimeRemaining))||' (Total: '||TimeString(DeltaTime+TimeRemaining)||')'
				else	Say End': 'G'/'IGateCount':'Image'/'ImageCount':Processing 'GateID' 'PathCount' Paths NonNumeric('TimeRemaining')'
			end

			Do P = 1 to PathCount
			if Images.Image.$DirectOnly = 0 | RxGetIndexValue('PATHS','Paths',P,'IGate') = 1 then
			do
				if SYMBOL('Paths.P.$IGate') = 'LIT' then	/* First time only */
				do
					Paths.P.$IGate = RxGetIndexValue('PATHS','Paths',P,'IGate')
					Paths.P.$ToStationID = RxGetIndexValue('PATHS','Paths',P,'ToStationID')
					Paths.P.$Relay = RxGetIndexValue('PATHS','Paths',P,'Relay')
					Paths.P.$FromStationID = RxGetIndexValue('PATHS','Paths',P,'FromStationID')
					Paths.P.$FromLatitude = RxGetIndexValue('PATHS','Paths',P,'From.Latitude')
					Paths.P.$FromLongitude = RxGetIndexValue('PATHS','Paths',P,'From.Longitude')
					Paths.P.$ToLatitude = RxGetIndexValue('PATHS','Paths',P,'To.Latitude')
					Paths.P.$ToLongitude = RxGetIndexValue('PATHS','Paths',P,'To.Longitude')
					Paths.P.$Distance = RxGetIndexValue('PATHS','Paths',P,'Distance')
				end

				FromID = Paths.P.$FromStationID
				FromLat = Paths.P.$FromLatitude
				FromLon = Paths.P.$FromLongitude
				FromRelay = Paths.P.$Relay
				ToIGate = Paths.P.$IGate
				ToID = Paths.P.$ToStationID
				ToLat = Paths.P.$ToLatitude
				ToLon = Paths.P.$ToLongitude
				Distance = Paths.P.$Distance
				FromInRange = InRange(FromLat, FromLon, Image)
				ToInRange = InRange(ToLat, ToLon, Image)

				if LEFT(FromID,4) = 'WIDE' & LEFT(FromID,5) = 'RELAY' then
					Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':Invalid From 'FromID' at 'FORMAT(FromLat,,3)' 'FORMAT(FromLon,,3)
				else if LEFT(ToID,4) = 'WIDE' & LEFT(ToID,5) = 'RELAY' then
					Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':Invalid To 'ToID' at 'FORMAT(ToLat,,3)' 'FORMAT(ToLon,,3)
				else if \ValidLatLon(FromLat, FromLon, Image) then
				do
					if SYMBOL('Invalid.FromID') = 'LIT' then
					do
						Invalid.FromID = 1
						Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':Invalid From Lat/Lon From 'FromID' ('Format(FromLat,,3)','Format(FromLon,,3)') to 'ToID' ('Format(ToLat,,3)','Format(ToLon,,3)')'
					end
				end
				else if \ValidLatLon(ToLat, ToLon, Image) then
				do
					if SYMBOL('Invalid.ToID') = 'LIT' then
					do
						Invalid.ToID = 1
						Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':Invalid To Lat/Lon From 'FromID' ('Format(FromLat,,3)','Format(FromLon,,3)') to 'ToID' ('Format(ToLat,,3)','Format(ToLon,,3)')'
					end
				end
				else if Distance > 400 then
				do
					if SYMBOL('Paths.P.$Warned') = 'LIT' then
					do
						Paths.P.$Warned = 1
						Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':Too Far From 'FromID' ('Format(FromLat,,3)','Format(FromLon,,3)') to 'ToID' ('Format(ToLat,,3)','Format(ToLon,,3)') Distance 'FORMAT(Distance,,2)' Miles'
					end
				end
				else if FromInRange | ToInRange then
				do
					if Distance > 300 & SYMBOL('Paths.P.$Warned') = 'LIT' then
					do
						Paths.P.$Warned = 1
						Say End': 'G'/'IGateCount':'Image'/'ImageCount':'P'/'PathCount':From 'FromID' to 'ToID' was 'FORMAT(Distance,,2)' Miles!'
					end

					fx = LonToX(FromLon, Image)
					fy = LatToY(FromLat, Image)
					tx = LonToX(ToLon, Image)
					ty = LatToY(ToLat, Image)

					if ToInRange then
					do
						if ToIGate = 1 then
						do
							if SYMBOL('Gates.Image.0.ToID') = 'LIT' then
							do
								GC = Gates.Image.0 + 1
								Gates.Image.0 = GC
								Gates.Image.0.ToID = GC
								Gates.Image.GC.$ID = ToID
								Gates.Image.GC.$xPos = tx
								Gates.Image.GC.$yPos = ty
							end
						end
						else	/* Must be to a relay */
						do
							if SYMBOL('Relays.Image.0.ToID') = 'LIT' then
							do
								RC = Relays.Image.0 + 1
								Relays.Image.0 = RC
								Relays.Image.0.ToID = 1
								Relays.Image.RC.$ID = ToID
								Relays.Image.RC.$xPos = tx
								Relays.Image.RC.$yPos = ty
							end
							else Relays.Image.0.ToID = Relays.Image.0.ToID + 1
						end
					end

					if FromInRange then
					do
						if FromRelay = 1 then
						do
							if SYMBOL('Relays.Image.0.FromID') = 'LIT' then
							do
								RC = Relays.Image.0 + 1
								Relays.Image.0 = RC
								Relays.Image.0.FromID = 1
								Relays.Image.RC.$ID = FromID
								Relays.Image.RC.$xPos = fx
								Relays.Image.RC.$yPos = fy
							end
							else Relays.Image.0.FromID = Relays.Image.0.FromID + 1
						end
					end

					if Images.Image.$MoverDots then
					do
						if FromRelay = 0 & FromInRange then
						do
							if SYMBOL('DIDDOT.Image.fx.fy') = 'LIT' then
							do
								DIDDOT.Image.fx.fy = 1
								Call GdImageFilledRectangle(Images.Image.$img, fx-2, fy-2, fx+2, fy+2, Images.Image.$black)
								Call GdImageFilledRectangle(Images.Image.$img, fx-1, fy-1, fx+1, fy+1, Images.Image.$red)
							end
						end
					end
					else if Images.Image.$Infrastructure = 0 | FromRelay=1 then
					do
						Call GdImageLine(Images.Image.$img,fx,fy,tx,ty,Images.Image.$red)
					end
				end
			End	/* if DirectOnly */
			End	/* Do P = 1 to PathCount */
		end	/* in range */
	end	/* RxStatus Success & Do Image */
	else
	do
		Zero.GateID = 1
		Say End': 'G'/'IGateCount':QueryUniquePaths('GateID') Got 0'
	end
	else Say End': 'G'/'IGateCount':QueryUniquePaths('GateID') Failed with 'RX.INTERRM
   End	/* Do G = 1 to GateCount */

   Do Image = 1 to ImageCount

	Call GdImageSetThickness(Images.Image.$img, 1);	/* For redundancy */
/*
	And label the Relays ON TOP OF the lines
*/
	Font = 'GDFONTGIANT'
	Font = 'GDFONTLARGE'
	Font = 'GDFONTMEDIUMBOLD'
	Font = 'GDFONTSMALL'
	Font = 'GDFONTTINY'

	Say End': 'Image'/'ImageCount':Labelling 'Relays.Image.0' Potential Relays'
	fH = GdFontGetHeight(Font); fH2 = FORMAT(fH/2,,0)
	fW = GdFontGetWidth(Font); fW2 = FORMAT(fW/2,,0)
	fS = fH
	Do G = Relays.Image.0 to 1 by -1
		RelayID = Relays.Image.G.$ID
		if SYMBOL('Gates.Image.0.RelayID') = 'LIT' then
		do
			x = Relays.Image.G.$xPos
			y = Relays.Image.G.$yPos

			if Images.Image.$BigDotRelays then
			do
/* Say ' 'G'/'RC':'RelayID' hit 'Relays.Image.0.RelayID */
				Call GdImageFilledEllipse(Images.Image.$img, x, y, fH, fH, Images.Image.$black);
				/* Call GdImageFilledEllipse(Images.Image.$img, x, y, format(FH*9/10,,0), format(FH*9/10,,0), Images.Image.$red); */
				Call GdImageFilledEllipse(Images.Image.$img, x, y, trunc(FH*8/10), trunc(FH*8/10), Images.Image.$pink);
				Call GdImageFilledEllipse(Images.Image.$img, x, y, format(FH*2/10,,0), format(FH*2/10,,0), Images.Image.$white);
				if Images.Image.$LabelRelays then
				do
					Call GdImageFilledRectangle(Images.Image.$img, x+fW2, y-fH2, x+LENGTH(Relays.Image.G.$ID)*fW+fW2+1, y+fH-fH2, Images.Image.$white);
					Call GdImageString(Images.Image.$img, Font, x+fW2+1, y-fH2, RelayID, Images.Image.$black)
				end
			end
			else if Images.Image.$SmallDotRelays then
			do
				Call GdImageFilledRectangle(Images.Image.$img, x-2, y-2, x+2, y+2, Images.Image.$black)
				Call GdImageFilledRectangle(Images.Image.$img, x-1, y-1, x+1, y+1, Images.Image.$green)
			end
		end
	End
/*
	And label the IGates ON TOP OF the lines
*/
	Font = 'GDFONTGIANT'
	Font = 'GDFONTLARGE'
	Font = 'GDFONTMEDIUMBOLD'
	Font = 'GDFONTSMALL'
	/* Font = 'GDFONTTINY' */

	Say End': 'Image'/'ImageCount':Labelling 'Gates.Image.0' IGates'
	fH = GdFontGetHeight(Font); fH2 = FORMAT(fH/2,,0)
	fW = GdFontGetWidth(Font); fW2 = FORMAT(fW/2,,0)
	fS = fH
	Do G = Gates.Image.0 to 1 by -1
		x = Gates.Image.G.$xPos
		y = Gates.Image.G.$yPos

		if Images.Image.$BigDotGates then
		do
			GateID = Gates.Image.G.$ID
			Call GdImageFilledEllipse(Images.Image.$img, x, y, fH, fH, Images.Image.$black);
			Call GdImageFilledEllipse(Images.Image.$img, x, y, trunc(FH*8/10), trunc(FH*8/10), Images.Image.$red);
			Call GdImageFilledEllipse(Images.Image.$img, x, y, format(FH*5/10,,0), format(FH*5/10,,0), Images.Image.$pink);
			Call GdImageFilledEllipse(Images.Image.$img, x, y, format(FH*2/10,,0), format(FH*2/10,,0), Images.Image.$white);
			if Images.Image.$LabelGates then
			do
				Call GdImageFilledRectangle(Images.Image.$img, x+fW2, y-fH2, Gates.Image.G.$xPos+LENGTH(GateID)*fW+fW2+1, Gates.Image.G.$yPos+fH-fH2, Images.Image.$white);
				Call GdImageString(Images.Image.$img, Font, x+fW2+1, y-fH2, GateID, Images.Image.$black)
			end
		end
		else if Images.Image.$SmallDotGates then
		do
			Call GdImageFilledRectangle(Images.Image.$img, x-3, y-3, x+3, y+3, Images.Image.$black)
			Call GdImageFilledRectangle(Images.Image.$img, x-1, y-1, x+1, y+1, Images.Image.$palegreen)
		end
	End

	Font = 'GDFONTGIANT'
	fH = GdFontGetHeight(Font); fH2 = FORMAT(fH/2,,0)
	fW = GdFontGetWidth(Font); fW2 = FORMAT(fW/2,,0)
	fS = fH

	Text = Start' to 'End
	xPos = format((Images.Image.$xSize-LENGTH(Text)*fW)/2,,0)
	yPos = Images.Image.$ySize - fH
	Call GdImageFilledRectangle(Images.Image.$img, xPos+fW2, yPos-fH2, xPos+LENGTH(Text)*fW+fW2+1, yPos+fH-fH2, Images.Image.$white);
	Call GdImageString(Images.Image.$img, Font, xPos+fW2+1, yPos-fH2, Text, Images.Image.$black)

	xPos = format((Images.Image.$xSize-LENGTH(Images.Image.$Title)*fW)/2,,0)
	yPos = Images.Image.$ySize - fH * 2 - fH2
	Call GdImageFilledRectangle(Images.Image.$img, xPos+fW2, yPos-fH2, xPos+LENGTH(Images.Image.$Title)*fW+fW2+1, yPos+fH-fH2, Images.Image.$white);
	Call GdImageString(Images.Image.$img, Font, xPos+fW2+1, yPos-fH2, Images.Image.$Title, Images.Image.$black)
   /*
    * Save the image...
    */
	if Images.Image.$iName \= '' & LEFT(Images.Image.$iName,1) \= '-' then Images.Image.$iName = '-'Images.Image.$iName
	Output = 'Act-'||Start||'-'||End||Images.Image.$iName||'('||Images.Image.$RenderStyle||')'||'-'||Images.Image.$Zoom||'.png'
	Output = TRANSLATE(Output,'...',':\/')
	Say End': 'Image'/'ImageCount':Saving 'Output
	call GdImagePNG(Images.Image.$img, Output)
	Say End': 'Image'/'ImageCount':Destroy stitched image'
	call GdImageDestroy(Images.Image.$img)
	Say End': 'Image'/'ImageCount':Output is in 'Output' (Zoom 'Images.Image.$Zoom')'
	Images.Image.$Output = Output

	End
Return

ValidLatLon:
Parse Arg vllLat, vllLon
	return vllLat >= -90 & vllLat <= 90 & vllLon >= -180 & vllLon <= 180 & (vllLat \=0 | vllLon \= 0)

InRange:
Parse Arg irLat, irLon, Image
	return irLat >= Images.Image.$MinLat & irLat <= Images.Image.$MaxLat & irLon >= Images.Image.$MinLon & irLon <= Images.Image.$MaxLon

LonToX:
Parse Arg ltxLong, Image
	return format(((ltxLong-Images.Image.$MinLon)*(Images.Image.$xsize/(Images.Image.$MaxLon-Images.Image.$MinLon)))+0.5,,0)

LatToY:
Parse Arg ltyLat, Image
	Do ltyI = 1 to Images.Image.$TileCount
		ltyStart = Images.Image.$Tiles.ltyI.$StartLat
		ltyEnd = Images.Image.$Tiles.ltyI.$EndLat
		if ltyLat >= ltyStart & ltyLat <= ltyEnd then
		do
			return (ltyI-1)*256 + 256-format(((ltyLat-ltyStart)*(256/(ltyEnd-ltyStart)))+0.5,,0)
		end
	End
	return Images.Image.$ysize - format(((ltyLat-Images.Image.$MinLat)*(Images.Image.$ysize/(Images.Image.$MaxLat-Images.Image.$MinLat)))+0.5,,0)

TimeString:
PARSE ARG InTimeDelta
do
	TimeStringOut = ''
if \Datatype(InTimeDelta,'N') then Say 'TimeString: NonNumeric 'InTimeDelta
	else
	do
		Days = TRUNC((InTimeDelta / 86400),0)
		Hrs  = TRUNC(((InTimeDelta - (Days * 86400))/3600),0)
		Mins = TRUNC((((InTimeDelta - (Days * 86400)) - (Hrs * 3600))/60),0)
		Secs = TRUNC(InTimeDelta - (Hrs * 3600) - (Mins * 60))

		if Days = 0 then
			Days = ''
		else
			Days = Days||'D '

		TimeStringOut = Days||RIGHT(Hrs,2,'0')||':'||RIGHT(Mins,2,'0')||':'RIGHT(Secs,2,'0')
	end
end
return TimeStringOut

-->
</script>

