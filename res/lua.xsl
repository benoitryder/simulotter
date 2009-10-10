<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="2.0"
  xmlns:xhtml="http://www.w3.org/1999/xhtml"
  xmlns="http://www.w3.org/1999/xhtml"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  exclude-result-prefixes="xhtml xsl xs">

  <xsl:output method="xml"
    version="1.0"
    encoding="UTF-8"
    doctype-public="-//W3C//DTD XHTML 1.1//EN"
    doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
    indent="yes"/>

  <xsl:template match="/">
    <html>
      <head>
        <title>SimulOtter Lua bindings</title>
        <style type="text/css">
          body * { margin: 0; font-family: sans-serif; }
          a { text-decoration: none; }

          h2 {
            font-size: 150%;
            margin: 3ex 0 1.5ex 0;
            font-weight: bold;
            padding: 3px 0 2px 0;
            background-color: #f8f8f8;
            border-top: 2px solid black;
            border-bottom: 1px solid black;
            text-indent: 1.5em;
          }
          h3 {
            font-size: 130%;
            background-color: #f0f0f0;
            text-indent: 0.5em;
            margin-top: 2ex;
            padding: 2px;
          }
          h4 {
            font-size: 120%;
            background-color: #f0f0f0;
            text-indent: 0.7em;
          }

          h2 span.text, h3 span.text {
            font-size: 80%;
          }
          h4 span.text {
            font-size: 90%;
          }
          h2 .name, h3 .name, h4 .name {
            color: #0000b0;
          }

          h2 span.module, h3 span.class, h3 span.object, h3 span.alias, h4 span.field {
            margin-right: .3em;
          }
          h3 span.base, h3 span.redirect {
            margin-left: 2em;
            margin-right: 0.5em;
          }

          code {
            font-family: monospace;
          }
          strong {
            font-weight: bold;
          }

          .proto, .type, .name, .proto .user {
            font-family: monospace;
          }
          .value .name, .function .name, .constructor .name {
            font-weight: bold;
          }
          .type {
            font-style: italic;
          }
          p.desc {
            margin-left: 1em;
          }
          table .desc {
            padding-left: 1em;
          }
          .class.desc, .object.desc {
            border: 1px solid #f0f0f0;
            padding: 0.5ex 0.5em 0.5ex 0.5em;
            margin-left: 0;
          }
          .field.desc {
            padding: 0.5ex 0.5em 0.5ex 0.5em;
            margin-left: 0;
          }

          div.function {
            margin: 1ex 0 0.5ex 1em;
            border-top: 1px solid #f0f0f0;
            padding-top: 2px;
          }
          .function .proto {
            margin-bottom: 0.5ex;
          }
          .function .proto .user {
            background-color: #eeeeee;
            color: #800000;
            font-weight: bold;
            padding: 0 .5em;
            margin-left: 1em;
          }
          div.field {
            margin: 1ex 0 0.5ex 1em;
            border: 1px solid #f0f0f0;
          }

          table.values {
            margin-left: 1em;
          }
          table.values td {
            vertical-align: top;
          }
          tr.value td.type {
            text-align: right;
            padding-right: 0.5em;
          }

          span.toggler {
            font-family: monospace;
            color: #444444;
            padding: 0.3ex 0.3em;
          }

        </style>
        <script type="text/javascript">

          function cOpen( id )
          {
            var content = document.getElementById('content_'+id);
            var toggler = document.getElementById('toggler_'+id);
            toggler.innerHTML = '-';
            content.style.display = 'block';
          }
          function cClose( id )
          {
            var content = document.getElementById('content_'+id);
            var toggler = document.getElementById('toggler_'+id);
            toggler.innerHTML = '+';
            content.style.display = 'none';
          }
          function cToggle( id )
          {
            var toggler = document.getElementById('toggler_'+id);
            if( toggler.innerHTML == '+' )
              cOpen(id);
            else
              cClose(id);
          }
          function cOpenAndMove( id )
          {
            var s = '';
            var a = id.split('.');
            for(var i=0; i &lt; a.length; i++)
            {
              if( s != '' )
                s += '.';
              s += a[i]
              cOpen(s);
            }
            location.hash = id;
          }

        </script>

      </head>
      <body>
        <xsl:apply-templates />
      </body>
    </html>
  </xsl:template>

  <xsl:template match="lua">
    <xsl:apply-templates select="module"/>
  </xsl:template>


  <xsl:template match="module">
    <div class="module">
      <h2>
        <xsl:call-template name="toggler"><xsl:with-param name="name" select="@name"/></xsl:call-template>
        <span class="text module"><xsl:text>Module </xsl:text></span>
        <span class="name"><xsl:value-of select="@name"/></span>
      </h2>
      <xsl:call-template name="toggle_content">
        <xsl:with-param name="name" select="@name"/>
        <xsl:with-param name="content">
          <p class="module desc"><xsl:call-template name="xhtml"/></p>
          <xsl:apply-templates select="object"/>
          <xsl:call-template name="values"/>
          <xsl:apply-templates select="class"/>
          <xsl:apply-templates select="alias"/>
        </xsl:with-param>
      </xsl:call-template>
    </div>
  </xsl:template>

  <xsl:template match="object">
    <div class="object">
      <xsl:variable name="name"><xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/></xsl:variable>
      <h3>
        <xsl:call-template name="toggler"><xsl:with-param name="name" select="$name"/></xsl:call-template>
        <span class="text object"><xsl:text>Object </xsl:text></span>
        <span class="name"><xsl:value-of select="@name"/></span>
      </h3>
      <xsl:call-template name="toggle_content">
        <xsl:with-param name="name" select="$name"/>
        <xsl:with-param name="content">
          <p class="object desc"><xsl:call-template name="xhtml"/></p>
          <xsl:apply-templates select="function"/>
          <xsl:apply-templates select="field"/>
          <xsl:call-template name="values"/>
        </xsl:with-param>
      </xsl:call-template>
    </div>
  </xsl:template>

  <xsl:template match="field">
    <div class="field">
      <xsl:variable name="name"><xsl:value-of select="../../@name"/>.<xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/></xsl:variable>
      <h4>
        <xsl:call-template name="toggler"><xsl:with-param name="name" select="$name"/></xsl:call-template>
        <span class="text field"><xsl:text>Field </xsl:text></span>
        <span class="name"><xsl:value-of select="@name"/></span>
      </h4>
      <xsl:call-template name="toggle_content">
        <xsl:with-param name="name" select="$name"/>
        <xsl:with-param name="content">
          <p class="field desc"><xsl:call-template name="xhtml"/></p>
          <xsl:call-template name="values"/>
        </xsl:with-param>
      </xsl:call-template>
    </div>
  </xsl:template>

  <xsl:template name="values">
    <table class="values">
      <xsl:apply-templates select="value"/>
    </table>
  </xsl:template>

  <xsl:template match="value">
    <tr class="value">
      <td class="type">
        <xsl:value-of select="@type"/>
      </td>
      <td class="name">
        <xsl:value-of select="@name"/>
      </td>
      <td class="desc"><xsl:call-template name="xhtml"/></td>
    </tr>
  </xsl:template>

  <xsl:template match="class">
    <div class="class">
      <xsl:variable name="name"><xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/></xsl:variable>
      <h3>
        <xsl:call-template name="toggler"><xsl:with-param name="name" select="$name"/></xsl:call-template>
        <span class="text class"><xsl:text>Class </xsl:text></span>
        <a class="name" name="{$name}"><xsl:value-of select="@name"/></a>
        <xsl:if test="@base != ''">
          <xsl:variable name="base">
            <xsl:choose>
              <xsl:when test="contains(@base,'.')"><xsl:value-of select="@base"/></xsl:when>
              <xsl:otherwise><xsl:value-of select="../@name"/>.<xsl:value-of select="@base"/></xsl:otherwise>
            </xsl:choose>
          </xsl:variable>
          <span class="base text"><xsl:text>base</xsl:text></span>
          <a class="base name" href="javascript:cOpenAndMove('{$base}');"><xsl:value-of select="@base"/></a>
        </xsl:if>
      </h3>
      <xsl:call-template name="toggle_content">
        <xsl:with-param name="name" select="$name"/>
        <xsl:with-param name="content">
          <p class="class desc"><xsl:call-template name="xhtml" /></p>
          <xsl:apply-templates select="field"/>
          <xsl:call-template name="values"/>
          <xsl:apply-templates select="constructor"/>
          <xsl:apply-templates select="function"/>
        </xsl:with-param>
      </xsl:call-template>
    </div>
  </xsl:template>

  <xsl:template match="alias">
    <div class="alias">
      <xsl:variable name="name"><xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/></xsl:variable>
      <h3>
        <span class="text alias"><xsl:text>Alias </xsl:text></span>
        <span class="name"><xsl:value-of select="@name"/></span>
        <span class="text redirect"><xsl:text>redirect to</xsl:text></span>
        <xsl:variable name="target">
          <xsl:choose>
            <xsl:when test="contains(@target,'.')"><xsl:value-of select="@target"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="../@name"/>.<xsl:value-of select="@target"/></xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <a class="target name" href="javascript:cOpenAndMove('{$target}');"><xsl:value-of select="@target"/></a>
      </h3>
      <xsl:call-template name="toggle_content">
        <xsl:with-param name="name" select="$name"/>
        <xsl:with-param name="content">
          <p class="object desc"><xsl:call-template name="xhtml"/></p>
          <xsl:apply-templates select="function"/>
          <xsl:call-template name="values"/>
        </xsl:with-param>
      </xsl:call-template>
    </div>
  </xsl:template>

  <xsl:template match="function|constructor">
    <div class="function">
      <p class="proto">
        <xsl:for-each select="return">
          <span class="type"><xsl:value-of select="@type"/></span>
          <xsl:if test="position()!=last()"><xsl:text>, </xsl:text></xsl:if>
        </xsl:for-each>
        <xsl:text> </xsl:text>
        <span class="name">
          <xsl:choose>
            <xsl:when test="@name"><xsl:value-of select="@name"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="../@name"/></xsl:otherwise>
          </xsl:choose>
        </span>
        <xsl:text> ( </xsl:text>
        <xsl:for-each select="arg">
          <xsl:if test="@required = '0'"><xsl:text> [ </xsl:text></xsl:if>
          <span class="type"><xsl:value-of select="@type"/></span>
          <xsl:if test="@name">
            <xsl:text> </xsl:text>
            <span class="name"><xsl:value-of select="@name"/></span>
          </xsl:if>
          <xsl:if test="@required = '0'"><xsl:text> ] </xsl:text></xsl:if>
          <xsl:if test="position()!=last()"><xsl:text>, </xsl:text></xsl:if>
        </xsl:for-each>
        <xsl:text> )</xsl:text>
        <xsl:if test="@user = '1'"><span class="user"><xsl:text>user func</xsl:text></span></xsl:if>
      </p>
      <p class="desc"><xsl:call-template name="xhtml"/></p>
    </div>
  </xsl:template>


  <xsl:template name="toggler">
    <xsl:param name="name"/>
    <span class="toggler" id="toggler_{$name}" onclick="cToggle('{$name}');">-</span>
  </xsl:template>

  <xsl:template name="toggle_content">
    <xsl:param name="name"/>
    <xsl:param name="content"/>
    <div class="content" id="content_{$name}"><xsl:copy-of select="$content"/></div>
  </xsl:template>


  <xsl:template name="xhtml">
    <xsl:apply-templates select="br|tt|b|em|text()"/>
  </xsl:template>
  <xsl:template match="br"><br/></xsl:template>
  <xsl:template match="tt"><code><xsl:apply-templates/></code></xsl:template>
  <xsl:template match="b"><strong><xsl:apply-templates/></strong></xsl:template>
  <xsl:template match="em"><em><xsl:apply-templates/></em></xsl:template>

</xsl:stylesheet>
