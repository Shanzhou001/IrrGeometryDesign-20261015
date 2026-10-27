param([Parameter(Mandatory=$true)][string]$DataPath)
$ErrorActionPreference='Stop'
$data=Get-Content -LiteralPath $DataPath -Raw -Encoding UTF8 | ConvertFrom-Json
$audit=New-Object System.Collections.Generic.List[object]
function RGB([string]$hex) {
    $value=$hex.TrimStart('#')
    return [int]([Convert]::ToInt32($value.Substring(0,2),16) + 256*[Convert]::ToInt32($value.Substring(2,2),16) + 65536*[Convert]::ToInt32($value.Substring(4,2),16))
}
function StyleText($shape,$element,[bool]$cellMode=$false) {
    $frame=$shape.TextFrame
    $frame.WordWrap=-1
    if(-not $cellMode){$frame.AutoSize=0}
    $frame.MarginLeft=0; $frame.MarginRight=0; $frame.MarginTop=0; $frame.MarginBottom=0
    $range=$frame.TextRange; $range.Text=[string]$element.value
    $range.Font.Name=[string]$data.font
    try {$range.Font.NameFarEast=[string]$data.farEastFont} catch {}
    $range.Font.Size=[single]$element.size
    $range.Font.Bold=0; if($element.bold){$range.Font.Bold=-1}
    $range.Font.Color.RGB=(RGB $element.color)
    $range.ParagraphFormat.Bullet.Visible=0
    $range.ParagraphFormat.SpaceBefore=0; $range.ParagraphFormat.SpaceAfter=2
    $range.ParagraphFormat.LineRuleWithin=-1
    $range.ParagraphFormat.SpaceWithin=[single]([double]$element.size*1.2)
    $range.ParagraphFormat.Alignment=1
}
function AddElement($slide,$element) {
    if($element.type -eq 'text') {
        $shape=$slide.Shapes.AddTextbox(1,[single]$element.x,[single]$element.y,[single]$element.w,[single]$element.h)
        [void](StyleText $shape $element)
        $shape.Left=[single]$element.x; $shape.Top=[single]$element.y
        $shape.Width=[single]$element.w; $shape.Height=[single]$element.h
    } elseif($element.type -eq 'image') {
        Add-Type -AssemblyName System.Drawing
        $bitmap=[Drawing.Image]::FromFile([string]$element.path)
        $ratio=$bitmap.Width/[double]$bitmap.Height; $bitmap.Dispose()
        if($element.cropRight -gt 0) {
            $shape=$slide.Shapes.AddPicture([string]$element.path,0,-1,[single]$element.x,[single]$element.y,-1,-1)
            $originalWidth=$shape.Width
            $shape.PictureFormat.CropRight=[single]($originalWidth*[double]$element.cropRight)
            $shape.LockAspectRatio=0
            $shape.Width=[single]$element.w; $shape.Height=[single]$element.h
            $shape.Left=[single]$element.x; $shape.Top=[single]$element.y
        } else {
            $width=[double]$element.w; $height=$width/$ratio
            if($height -gt [double]$element.h){$height=[double]$element.h;$width=$height*$ratio}
            $left=[double]$element.x+([double]$element.w-$width)/2
            $top=[double]$element.y+([double]$element.h-$height)/2
            $shape=$slide.Shapes.AddPicture([string]$element.path,0,-1,[single]$left,[single]$top,[single]$width,[single]$height)
        }
        $shape.AlternativeText=[string]$element.alt
    } elseif($element.type -eq 'table') {
        $shape=$slide.Shapes.AddTable($element.rows.Count,$element.widths.Count,[single]$element.x,[single]$element.y,[single]$element.w,[single]$element.h)
        $table=$shape.Table
        for($column=1;$column -le $element.widths.Count;$column++){$table.Columns.Item($column).Width=[single]$element.widths[$column-1]}
        for($row=1;$row -le $element.rows.Count;$row++) {
            $table.Rows.Item($row).Height=[single]$element.heights[$row-1]
            for($column=1;$column -le $element.widths.Count;$column++) {
                $cell=$table.Cell($row,$column).Shape
                $value=[string]$element.rows[$row-1][$column-1]
                $cellElement=[PSCustomObject]@{value=$value;size=$element.size;color=$data.colors.body;bold=$false}
                if($row -eq 1){$cellElement.color=$data.colors.white;$cellElement.bold=$true}
                [void](StyleText $cell $cellElement $true)
                $range=$cell.TextFrame.TextRange
                $cell.TextFrame.MarginLeft=8;$cell.TextFrame.MarginRight=8;$cell.TextFrame.MarginTop=2;$cell.TextFrame.MarginBottom=2
                $cell.TextFrame.VerticalAnchor=3
                if($element.name -eq 'editable-viking-means'){$range.ParagraphFormat.Alignment=2}
                $cell.Fill.Solid()
                $fill=$data.colors.white
                if($row -eq 1){$fill=$data.colors.teal} elseif($row%2 -eq 0){$fill=$data.colors.light}
                $cell.Fill.ForeColor.RGB=(RGB $fill)
                for($side=1;$side -le 4;$side++) {
                    $border=$table.Cell($row,$column).Borders.Item($side)
                    $border.ForeColor.RGB=(RGB $data.colors.line);$border.Weight=0.5
                }
            }
        }
    } else {throw ('Unknown element '+$element.type)}
    $shape.Name=[string]$element.name
    $item=[ordered]@{slide=$slide.SlideIndex;name=$element.name;kind=$element.type;left=$shape.Left;top=$shape.Top;width=$shape.Width;height=$shape.Height}
    if($element.type -eq 'text') {
        try {$item.boundHeight=$shape.TextFrame.TextRange.BoundHeight;$item.boundWidth=$shape.TextFrame.TextRange.BoundWidth} catch {}
        $item.text=$element.value
    }
    $audit.Add([PSCustomObject]$item)
}

$application=$null; $presentation=$null
try {
    $application=New-Object -ComObject PowerPoint.Application
    $application.DisplayAlerts=1
    $presentation=$application.Presentations.Add(0)
    $presentation.PageSetup.SlideWidth=[single]$data.width
    $presentation.PageSetup.SlideHeight=[single]$data.height
    foreach($slideData in $data.slides) {
        $slide=$presentation.Slides.Add($presentation.Slides.Count+1,12)
        $slide.FollowMasterBackground=0;$slide.Background.Fill.Solid();$slide.Background.Fill.ForeColor.RGB=(RGB '#FFFFFF')
        $slide.Name=[string]$slideData.name
        foreach($element in $slideData.elements){AddElement $slide $element}
        $notes=$null
        foreach($placeholder in $slide.NotesPage.Shapes.Placeholders) {
            if($placeholder.PlaceholderFormat.Type -eq 2){$notes=$placeholder;break}
        }
        if($null -eq $notes){$notes=$slide.NotesPage.Shapes.AddTextbox(1,30,100,600,400)}
        $notes.TextFrame.TextRange.Text=[string]$slideData.notes
    }
    $draft=Join-Path $data.build 'draft.pptx'
    if(Test-Path -LiteralPath $draft){Remove-Item -LiteralPath $draft}
    $presentation.SaveAs($draft,24)
    $previews=Join-Path $data.build 'preview';New-Item -ItemType Directory -Force -Path $previews | Out-Null
    for($i=1;$i -le $presentation.Slides.Count;$i++) {
        $presentation.Slides.Item($i).Export((Join-Path $previews ('slide_{0:D2}.png' -f $i)),'PNG',1600,900)
    }
    $audit | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $data.build 'layout_audit.json') -Encoding UTF8
    [PSCustomObject]@{status='draft_built';slides=$presentation.Slides.Count;draft=$draft;application=$application.Name;version=$application.Version} | ConvertTo-Json -Compress
} catch {
    Write-Output $_.ScriptStackTrace
    Write-Output $_.InvocationInfo.PositionMessage
    throw
} finally {
    if($null -ne $presentation){$presentation.Close();[void][Runtime.InteropServices.Marshal]::ReleaseComObject($presentation)}
    if($null -ne $application){if($application.Presentations.Count -eq 0){$application.Quit()};[void][Runtime.InteropServices.Marshal]::ReleaseComObject($application)}
}
