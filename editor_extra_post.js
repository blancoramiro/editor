
function call_draw()
{
	Module.ccall('draw', // name of C function
	null, // return type
	[], // argument types
	[]);
}

function canvas_resize()
{
	canvas.width = document.body.offsetWidth;
	canvas.height = 300; 
}

window.addEventListener('paste', function (e)
{
	if(e.clipboardData != false) {
		//console.log(e.clipboardData.getData('Text'));
		let chars = e.clipboardData.getData('Text').split('');
		//	    console.log(chars);

		for (var i = 0; i < chars.length; i++) {
			Module.ccall('paste_char', // name of C function
			'number', // return type
			['string'], // argument types
			[chars[i]]);
		}
		call_draw();
	}
}, false);

window.addEventListener("keypress", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("keydown", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("keyup", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("click", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("mousemove", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("scroll", function(/*e*/) { call_draw(); /*e.preventDefault();*/ return false });

window.addEventListener("resize", function(/*e*/) { canvas_resize(); return false });

