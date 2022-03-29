function show_item(item)
{
    item.style.display = '';
    item.style.visibility = 'visible';
}

function hide_item(item)
{
    item.style.display = 'none';
    item.style.visibility = 'hidden';
}

function applyFilter(filter, table_id, header_id, note_id)
{
    var table = document.getElementById(table_id);
    var header = document.getElementById(header_id);
    var note = document.getElementById(note_id);
    
    var pattern = filter.value.toLowerCase();
    
    var count = 0;
    for (var i = 0; row = table.rows[i]; i++)
    {
        if(row.id==header_id)
            continue;
        
        var show = 0;
        
        if(pattern=='') {
            show = 1;
        }
        else
        {
            var text = row.cells[0].innerHTML.toLowerCase();
            if(text.indexOf(pattern)!=-1) {
                show = 1;
            }
        }
        
        if(show)
        {
            show_item(row);
            count += 1;
        }
        else {
            hide_item(row);
        }
    }
    
    if(count==0)
    {
        hide_item(header);
        show_item(note);
    }
    else
    {
        show_item(header);
        hide_item(note);
    }
}
