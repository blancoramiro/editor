/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

const char* vertex_shader_text =
	"attribute vec2 vPos;"
	"varying vec2 tPos;"
	"void main(void)"
	"{"
	"    gl_Position = vec4(vPos, 0.0, 1.0);"
	"    tPos = vPos;"
	"}";


const char* fragment_shader_text =
#ifdef __EMSCRIPTEN__
	"precision highp float;"
#endif
	"varying vec2 tPos;"
	"uniform vec2 Mouse;"
	"uniform vec2 Cursor;"
	"uniform float CursorBlink;"
	"uniform vec4 Grid;"
	"uniform float GridYOffsset;"
	"uniform sampler2D Chars;"
	"uniform sampler2D Selection;"
	"uniform sampler2D Texture;"
	"float Char, Select;"
	"float xpos, ypos;"
	"vec4 Result;"
	"void main(void)"
	"{"
#ifdef CHAR_GRID
	"	if(mod(ceil(gl_FragCoord.x), Grid.z) == 0. || mod(ceil(gl_FragCoord.y+GridYOffsset), Grid.w) == 0.)"
	"	{"
	"		gl_FragColor = vec4(1., 0., 0., 0.);"
	"	}"
	"	else"
	"	{"
#endif
	"		xpos = ceil(gl_FragCoord.x*(1./Grid.z));"
	"		ypos = Grid.y-floor((gl_FragCoord.y+GridYOffsset)*(1./Grid.w));"
	"		Char = texture2D(Chars, vec2((1./Grid.x*xpos-1./Grid.x/2.)   , 1./Grid.y*(.5+ypos))).r*256.;"
	"		Select = texture2D(Selection, vec2((1./Grid.x*xpos-1./Grid.x/2.)   , 1./Grid.y*(.5+ypos))).r*256.;"
	"		Result = ceil(texture2D(Texture, vec2(((mod(gl_FragCoord.x, Grid.z)*14.)/Grid.z)/256.+(14./256.)*ceil(mod(Char,18.)), ((mod(gl_FragCoord.y+GridYOffsset, Grid.w)*18.)/Grid.w)/128.+(18./128.)*  ceil(6.-Char/18.)     )));"
	"		if((ypos == Cursor.y && xpos == Cursor.x && CursorBlink > 0.) || ypos == Mouse.y && xpos == Mouse.x)"
	"		gl_FragColor = vec4(1., 0., 0., 0.) - Result;"
	"		else gl_FragColor = (1. - Select) * (vec4(1., 0., 0., 0.) * Result) + Select * (vec4(1., 0., 0., 0.) * (1. - Result));"
#ifdef CHAR_GRID
	"	}"
#endif
	"}";

const char* fragment_shader_scroll_text =
#ifdef __EMSCRIPTEN__
	"precision highp float;"
#endif
	"uniform vec2 ScrollBar;"
	"void main(void)"
	"{"
	"      gl_FragColor = vec4(.08, .08, .08, .1) + vec4(.3, 0., 0., 0.) * vec4(ceil(clamp(gl_FragCoord.y - ScrollBar.y, 0., 1.)) - ceil(clamp(gl_FragCoord.y - ScrollBar.x, 0., 1.)));"
	"}";

