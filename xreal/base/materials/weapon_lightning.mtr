
lightningBoltNew
{
	cull disable
	{
		map models/weapons2/lightning/trail/h-trail1.tga
		blend add
		rgbGen wave sin 1 0.5 0 7.1 
		scale 1, 1
		scroll time * -1, 0
	}
	{
		map models/weapons2/lightning/trail/h-trail2.tga
		blend add
		rgbGen identity
		scroll time * -5, 0
		scale 0.4, 0
	}
}

models/weapons2/lightning/h_lg_spark
{
	deformVertexes autosprite
	{
		clampmap models/weapons2/lightning/h_spark1.tga
		blendfunc add
		rgbGen wave noise 0.4 0.6 0.5 15 
		tcMod rotate -276
		tcMod stretch sin 0.5 0.3 0.5 1 
	}
	{
		clampmap models/weapons2/lightning/h_spark1.tga
		blendfunc add
		rgbGen wave noise 0.7 0.7 0 19 
		tcMod stretch sin 0.6 0.3 0.3 2.4 
		tcMod rotate 200
	}
}

models/weapons2/lightning/fireflare1
{
	deformVertexes autosprite
	{
		clampmap models/weapons2/lightning/fireflare1.tga
		blendfunc add
		rgbGen wave noise 0.4 0.6 0.5 15 
		tcMod rotate -276
		tcMod stretch sin 0.5 0.3 0.5 1 
	}
	{
		clampmap models/weapons2/lightning/fireflare2.tga
		blendfunc add
		rgbGen wave noise 0.7 0.7 0 19 
		tcMod stretch sin 0.6 0.3 0.3 2.4 
		tcMod rotate 200
	}
	{
		clampmap models/weapons2/lightning/fireflare3.tga
		blendfunc add
		rgbGen wave noise 0 1 0.6 0.3 
		tcMod turb 0.01 0.01 0 25
		tcMod rotate 420
		tcMod stretch sin 0.6 0.4 0 3 
	}
}

models/weapons2/lightning/h_lg_body
{
	{
		stage diffuseMap
		map models/weapons2/lightning/h_lg_body.tga
		rgbGen lightingDiffuse
	}
	{
		stage bumpMap
		map models/weapons2/lightning/h_lg_body_n.tga
	}
	{
		stage specularMap
		map models/weapons2/lightning/h_lg_body_s.tga
	}
	{
		map models/weapons2/lightning/h_lg_bodylight.tga
		blendfunc blend
		rgbGen const ( 0.4 0 0.2 )
		alphaGen wave sin 0.4 0 0 1 
	}
	{
		map models/weapons2/lightning/h_lg_bodylight.tga
		blendfunc add
		rgbGen wave sin 0 1 0 0.5 
	}
}

models/weapons2/lightning/h_lg_cara
{
	{
		stage diffuseMap
		map models/weapons2/lightning/h_lg_cara.tga
		rgbGen lightingDiffuse
	}
	{
		stage bumpMap
		map models/weapons2/lightning/h_lg_cara_n.tga
	}
	{
		stage specularMap
		map models/weapons2/lightning/h_lg_cara_s.tga
	}
}

models/weapons2/lightning/h_lg_panel
{
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_1.tga
		color 0.129412, 0.258824, 0.258824, 1
	}
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_2.tga
		blend filter
		alphaFunc GT0
	}
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_4.tga
		blend filter
		alphaFunc GT0
		alphaGen wave sin 0 1 0 0.5
	}
	{
		animmap 25 models/weapons2/lightning/paneldisp/h_lg_panel_6.tga models/weapons2/lightning/paneldisp/h_lg_panel_7.tga models/weapons2/lightning/paneldisp/h_lg_panel_6.tga models/weapons2/lightning/paneldisp/h_lg_panel_6.tga models/weapons2/lightning/paneldisp/h_lg_panel_6.tga models/weapons2/lightning/paneldisp/h_lg_panel_8.tga models/weapons2/lightning/paneldisp/h_lg_panel_8.tga models/weapons2/lightning/paneldisp/h_lg_panel_6.tga 
		blend filter
		//color 1, 0, 0, 1
	}
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_5.tga
		blend filter
		scroll time * 1.5, 0
		alphaFunc GT0
		alphaGen wave square 0 1 0.5 2
	}
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_5.tga
		blend filter
		scroll time * -1, 0
		alphaFunc GT0
		alphaGen wave square 0 1 0 2
	}
	{
		map models/weapons2/lightning/paneldisp/h_lg_panel_3.tga
		blend filter
		alphaFunc GT0
		alphaGen wave sin 0 1 0 0.02
	}
	{
		map models/weapons2/lightning/h_lg_panel.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
}

/*
models/powerups/ammo/lighammo2
{
	{
		clampmap models/powerups/ammo/lighammo2.tga
		tcMod stretch sin 1.2 0.2 0 1 
		tcMod rotate -180
	}
	{
		map models/powerups/ammo/lighammo2b.tga
		blendfunc blend
	}
}
*/
