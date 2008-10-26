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
            margin: 2ex 0 1ex 0;
            font-weight: bold;
          }

          h3 {
            font-size: 150%;
            background-color: #f0f0f0;
            text-indent: 0.5em;
            margin-top: 2ex;
            padding: 2px;
          }
          h3 span.text {
            font-size: 80%;
            font-weight: normal;
          }
          h3 .name {
            color: #0000b0;
          }

          h3 span.class {
            margin-right: .3em;
          }
          h3 span.base {
            margin-left: 2em;
            margin-right: 0.5em;
          }

          .proto, .type, .name {
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
            padding: 0.5ex 0 0.5ex 0.5em;
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

          table.values {
            margin-left: 1em;
          }
          tr.value td.type {
            text-align: right;
            padding-right: 0.5em;
          }

        </style>
      </head>
      <body>
        <xsl:apply-templates />
      </body>
    </html>
  </xsl:template>

  <xsl:template match="lua">
    <h2>Globals</h2>
    <xsl:apply-templates select="globals"/>
    <h2>Classes</h2>
    <xsl:apply-templates select="class"/>
  </xsl:template>


  <xsl:template match="globals">
    <div id="globals" href="#globals">
      <xsl:apply-templates  select="object"/>
      <xsl:call-template name="values"/>
    </div>
  </xsl:template>

  <xsl:template match="object">
    <div class="object"><h3><span class="name"><xsl:value-of select="@name"/></span></h3>
      <p class="object desc"><xsl:call-template name="xhtml"/></p>
      <xsl:apply-templates  select="function"/>
      <xsl:call-template name="values"/>
    </div>
  </xsl:template>

  <xsl:template name="values">
    <table class="values">
      <xsl:apply-templates  select="value"/>
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
      <h3>
        <span class="text class"><xsl:text>Class </xsl:text></span>
        <a class="name" name="{@name}"><xsl:value-of select="@name"/></a>
        <xsl:if test="@base != ''">
          <span class="base text"><xsl:text>base</xsl:text></span>
          <a class="base name" href="#{@base}"><xsl:value-of select="@base"/></a>
        </xsl:if>
      </h3>
      <p class="class desc"><xsl:call-template name="xhtml" /></p>
      <xsl:apply-templates select="constructor"/>
      <xsl:apply-templates select="function"/>
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
      </p>
      <p class="desc"><xsl:call-template name="xhtml"/></p>
    </div>
  </xsl:template>

  <xsl:template name="xhtml">
    <xsl:apply-templates select="br|text()"/>
  </xsl:template>
  <xsl:template match="br"><br/></xsl:template>

</xsl:stylesheet>
