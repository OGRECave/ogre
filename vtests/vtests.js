function getChildElement(index)
{
	var ind = 0;
	for(var i = 0; i < this.childNodes.length; ++i)
	{
		if(this.childNodes[i].nodeType == 1)
		{
			if(index == ind)
			{
				return this.childNodes[i];
			}
			++ind;
		}
	}
	return null;
}

function getNumChildElements()
{
	var num = 0;
	for(var i = 0; i < this.childNodes.length; ++i)
	{
		if(this.childNodes[i].nodeType == 1)
			++num;
	}
	return num;
}

Element.prototype.getNumChildElements = getNumChildElements;
Element.prototype.getChildElement = getChildElement;

var entries;
var imgx = 0;
var imgy = 0;

window.onload = function()
{
	entries = [];

	// get res from the stat table (since chrome is iffy about reading img dimensions)
	var res = document.body.getElementsByTagName("table")[0].rows[3].
		getChildElement(1).innerHTML;
	imgx = parseInt(res);
	imgy = parseInt(res.split("x ")[1]);

	// insert buttons
	for(var i = 2, n = document.body.getNumChildElements(); i < n; ++i)
	{
		// find each image box:
		var content = document.body.getChildElement(i).getChildElement(1);
		var divs = content.getElementsByTagName("div");

		// for each image in the test entry
		for(var j = 0; j < divs.length; j+=3)
		{
			// create buttons
			var btn = document.createElement("button");
			btn.innerHTML = "Toggle Diff";
			btn.parentEntry = divs[j];
			btn.onclick = toggleDiff;
			var btn2 = document.createElement("button");
			btn2.innerHTML = "Swap Images";
			btn2.parentEntry = divs[j];
			btn2.onclick = swapImgs;

			// add buttons
			divs[j].insertBefore(btn, divs[j].getChildElement(2));
			divs[j].insertBefore(btn2, divs[j].getChildElement(2));

			// add to the list
			entries.push(divs[j]);
			divs[j].diffOn = false;
			divs[j].hasDiff = false;
		}
	}
};

function toggleDiff()
{
	var entry = this.parentEntry;
	var col1 = entry.getChildElement(0);
	var col2 = entry.getChildElement(1);
	var img1 = col1.getElementsByTagName("img")[0];
	var img2 = col2.getElementsByTagName("img")[0];

	var can = null;

	if(entry.hasDiff)
	{
		can = entry.getChildElement(2);

		if(entry.diffOn)
		{
			can.style.display = "none";
			col2.style.display = "block";
		}
		else
		{
			can.style.display = "block";
			col2.style.display = "none";
		}

		entry.diffOn = !entry.diffOn;
	}
	else
	{
		// hide original images
		col1.style.display = "none";
		col2.style.display = "none";

		// make a column for this to live in
		var column = document.createElement("div");
		column.className = "img_column";
		// and a header
		var header = document.createElement("h3");
		header.innerHTML = "Diff:";
		column.appendChild(header);

		// make a couple canvases so we can read image data
		var canvas1 = document.createElement("canvas");
		canvas1.width = imgx;
		canvas1.height = imgy;
		var canvas2 = document.createElement("canvas");
		canvas2.width = imgx;
		canvas2.height = imgy;

		// get ImageData from contexts
		var context = canvas1.getContext("2d");
		var context2 = canvas2.getContext("2d");
		context.drawImage(img1, 0, 0);
		context2.drawImage(img2, 0, 0);
		var data = context.getImageData(0, 0, imgx, imgy);
		var data2 = context2.getImageData(0, 0, imgx, imgy);
		var px1 = data.data;
		var px2 = data2.data;

		// do the diffing
		for(var i = 0, n = px1.length; i < n; i+=4)
		{
			px1[i] = px1[i] > px2[i] ? px1[i] - px2[i] : px2[i] - px1[i];
			px1[i+1] = px1[i+1] > px2[i+1] ? px1[i+1] - px2[i+1] : px2[i+1] - px1[i+1];
			px1[i+2] = px1[i+2] > px2[i+2] ? px1[i+2] - px2[i+2] : px2[i+2] - px1[i+2];
		}

		// save to one of the canvases
		context.putImageData(data, 0, 0);

		// and insert the canvas
		canvas1.style.width = "480px";
		column.appendChild(canvas1);
		entry.insertBefore(column, entry.getChildElement(2));

		entry.hasDiff = true;
		entry.diffOn = true;
		col1.style.display = "block";
	}
}

function swapImgs()
{
	var entry = this.parentEntry;
	var divs = entry.getElementsByTagName("div");
	for(var i = 0; i < divs.length; ++i)
	{
		if(divs[i].style.cssFloat == "right")
			divs[i].style.cssFloat = "left";
		else
			divs[i].style.cssFloat = "right";
	}
}
