// Ogre overlay scripts
template container BorderPanel(SS/Templates/BasicBorderPanel)
{
	left 0
	top 0
	width 1
	height 1
	material Core/StatsBlockCenter
    	border_size 0.06 0.06 0.06665 0.06665
    	border_material Core/StatsBlockBorder
    	border_topleft_uv     0.0000 0.0000 0.1914 0.2031
	border_top_uv         0.1914 0.0000 0.8086 0.2031
	border_topright_uv    0.8086 0.0000 1.0000 0.2031
	border_left_uv        0.0000 0.2031 0.1914 0.7852
	border_right_uv       0.8086 0.2031 1.0000 0.7852
	border_bottomleft_uv  0.0000 0.7852 0.1914 1.0000
        border_bottom_uv      0.1914 0.7852 0.8086 1.0000
	border_bottomright_uv 0.8086 0.7852 1.0000 1.0000
}

template container BorderPanel(SS/Templates/BasicSmallBorderPanel) : SS/Templates/BasicBorderPanel
{
    border_size 0.04 0.04 0.04665 0.04665
}


template element TextArea(SS/Templates/BasicText)
{
	font_name Ogre
	char_height 0.08
	colour_top 1 1 0
	colour_bottom 1 0.2 0.2
	left 0.03
	top 0.02
	width 0.12
	height 0.09
}

template container Panel(SS/Templates/BasicPanel)
{
	left 0 
	top 0
	width 1
	height 1
	material Core/StatsBlockCenter
}

