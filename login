

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
    <script type="text/javascript">window.ST=new Date().getTime();</script>
    <meta content="text/html; charset=UTF-8" http-equiv="content-type" />
    <meta http-equiv="X-UA-Compatible" content="IE=7" />
    <meta name="description" content="Dropbox is a free service that lets you bring your photos, docs, and videos anywhere and share them easily. Never email yourself a file again!"/>
    <meta name="keywords" content="online storage, free storage, file sharing, share files, awesome, cloud storage, online backup, cross platform, sync, sharing, mac, windows, os x, linux, backup, collaboration, file versioning, file revisions, remote access, undelete"/>
    <meta name="google-site-verification" content="TnuSyOnBMNmtugbpL1ZvW2PbSF9LKvoTzrvOGS9h-b0" />
    <meta name="google-site-verification" content="EZKIczQcM1-DVUMz8heu1dIhNtxNbLqbaA9-HbOnCQ4" />
    <meta name="norton-safeweb-site-verification" content="tz8iotmk-pkhui406y41y5bfmfxdwmaa4a-yc0hm6r0fga7s6j0j27qmgqkmc7oovihzghbzhbdjk-uiyrz438nxsjdbj3fggwgl8oq2nf4ko8gi7j4z7t78kegbidl4" />
    <title>Dropbox 
                - Log in
        - Simplify your life
    </title>

    <link rel="shortcut icon" href="/static/16890/images/favicon.ico"/>

    <link href="/static/16890/css/main.css" rel="stylesheet" type="text/css"/>
    <link href="/static/16890/css/lib.css" rel="stylesheet" type="text/css" media="screen"/>
    <link href="/static/16890/css/sprites.css" rel="stylesheet" type="text/css" media="screen"/>

    <link rel="apple-touch-icon" href="/static/16890/images/dropbox_webclip.png"/>
    <link rel="P3Pv1" href="/w3c/p3p.xml"/>
    


<script type="text/javascript">
var Constants = {
  BLOCK_CLUSTER: 'dl-web.dropbox.com',
  PUBSERVER: 'dl.dropbox.com',
  WEBSERVER: 'www.dropbox.com',
  block: 'dl-web.dropbox.com',
  protocol: 'https',
  uid: '',
  root_ns: '',
  SVN_REV: '16890',
  TOKEN: '',
  IS_PROD: 1,
  WIT_ENABLED: 0,
  upload_debug: false,
  tcn: 'touch',
  date_format: 'M/d/yyyy',
  referrer: '', 
  TWO_ITEM_LIST: '%(x)s and %(y)s',
  THREE_ITEM_LIST: '%(x)s, %(y)s and %(z)s',
  LOCALES: [["en", "English"], ["es", "Espa\u00f1ol"], ["fr", "Fran\u00e7ais"], ["de", "Deutsch"], ["ja", "\u65e5\u672c\u8a9e"]],
  USER_LOCALE: 'en'
};
</script>
    <script type="text/javascript" src="/static/16890/javascript/lib-mini.js"></script>
    <script type="text/javascript">
        function global_report_exception (e, f, l, tb, force) {
            if (!window.reported_exception || force) {
                var stack_str = "";
                try {
                    if (!tb) {
                        var stack = get_stack_rep();
                        stack.pop(); // remove global_report_exception
                        stack.pop(); // remove onerror handler
                        stack_str = stack.join("\n");
                    }
                } catch (e) { }
                var dbr = new Ajax.Request("/jse", {parameters: {'e': e, 'f': f || window.location.href, 'l': l, 'loc': window.location.href, 'ref': Constants.referrer, 'tb': tb || stack_str }});
                window.reported_exception = true;
           }
        }
        window.LoadedJsSuccessfully = false; // this constant gets set to true at the bottom of dropbox.js
        Event.observe(window, "load", function () {
            if (!window.LoadedJsSuccessfully) {
                global_report_exception("Failed to load javascript!");
            }
        });
        window.onerror = function (e, f, l) {
            global_report_exception(e, f, l);
        };
    </script>
    <script type="text/javascript" src="/static/16890/javascript/dropbox-mini.js"></script>

    <style type="text/css">
      .hny-idlotb { display: none; }
    </style>
    <style type="text/css">
    #login-table { width: 280px; } 
</style>
<script type="text/javascript" charset="utf-8">
    function fill_email () {
        var login_email = $("login-content").down("#email");
        var register_email = $("register-content").down("#email");
        
        if (!login_email) {
            return;
        }
        
        if (login_email.getValue() && !register_email.getValue()) {
            register_email.setValue(login_email.getValue());
        }
    }
</script>

</head>

<body class="en">

    

    <div id="modal-behind" style="display: none;"></div>

    <div id="modal" style="display: none;">
        <div id="modal-box">
            <a onclick="javascript: Modal.hide(); Event.stop(event); return false;" href="#" style="float: right;"><img src="/static/images/icons/icon_spacer.gif" alt="" border="0" class="sprite s_x" /></a>
            <h2 id="modal-title"></h2>
            <div id="modal-content">
            </div>
        </div>
    </div>

    <div id="modal-overlay" style="display: none;" onclick="Modal.hide(); Event.stop(event); return false;"></div>
    <div id="floaters"></div><div id="trash-can" style="display:none"></div><div id="grave-yard" style="display:none"></div>


    <div id="feedback-div" style="display: none">
        <div id="feedback">
            <form method="post" action="/feedback">
                Suggestions, ideas, bug reports, and comments are always welcome. If you'd like to interact with other Dropbox users, check out our <a id="forums_link">forums</a>.
                <script type="text/javascript">
                   Util.jag('forums_link', 'href', 'http://forums.dropbox.com');
                </script>
                <br/><br/>

                Email Address (optional)<br />
                <input type="text" name="feedback_email" value="" class="textinput act_as_block" id="feedback_email" /><br /><br />
                <textarea class="textinput act_as_block" name="content" id="feedback_textarea" rows="1" cols="1"></textarea>

                <div align="right">
                    <input type="text" name="email" class="hny-idlotb"/> 
                    <input type="submit" value="Send feedback" class="button" />
                </div>
            </form>
        </div>
    </div>
    <div id="translate-div" style="display:none; margin-bottom: -10px">
        <form id="translation-suggest-form" onsubmit="TranslationSuggest.submit_suggest(event); return false;" action='/translation_suggest' method="POST">
            <input type="hidden" name="locale" value="en" />
            <input type="hidden" name="locale_url" value="https://www.dropbox.com/login" />
            <input type="hidden" id="translation-msg-id" name="msg_id" value="" />
            <p class="clean">Please paste or type the improper translation</p>
            <div id="part-one">
                <p>
                    <span class="error-message" id="bad-i18n-text-error" style="display: none">We couldn't find that string on this page.</span>
                    <textarea name="bad_text" id="bad-i18n-text" class="act_as_block textinput" rows='3' cols='40'></textarea>
                </p>
                <div id="bad-i18n-text-complete" class="autocomplete"></div>
            </div>
            <div id="part-two">
                                    <div class="emo hotbox" id="translation-msg-display" style="margin-bottom: 10px"></div>
                  <p class="clean">Original English text</p>
                  <div class="green-hotbox" id="translation-orig-msg-display" style="margin-bottom: 10px"></div>
                  <p class="clean">Suggested translation</p>
                  <p><textarea name="suggested_text" class="textinput act_as_block" rows='3' cols='40'></textarea></p>
                  <p class="clean">Explanation of the problem</p>
                  <p><textarea name="explanation" class="textinput act_as_block" rows='3' cols='40'></textarea></p>
                  <p style="text-align: right; margin-bottom:0;">
                      <input type="button" class="button grayed" onclick="TranslationSuggest.start_wizard(event);return false;" value="Back" id="translation-back-button"/>
                      <input type="submit" class="button" onclick="TranslationSuggest.submit_suggest(event); return false;" value="Suggest Translation" />
                  </p>
            </div>
        </form>
    </div>

    <div id="server-error" class="notify server-er" style="display: none;">
        <table cellpadding="0" cellspacing="0">
            <tr>
                <td class="msg-td">
                    <span class="msg-span">There was a problem completing this request.</span>
                </td>
            </tr>
            <tr>
                <td>
                    <b class="rbottom"><b class="r0"></b><b class="r1"></b></b>
                </td>
            </tr>
        </table>
    </div>
    <div id="server-success" class="notify server-ok" style="display: none;">
        <table cellpadding="0" cellspacing="0">
            <tr>
                <td class="msg-td">
                    <span>Request completed successfully.</span>
                </td>
            </tr>
            <tr>
                <td>
                    <b class="rbottom"><b class="r0"></b><b class="r1"></b></b>
                </td>
            </tr>
        </table>
    </div>
<div id="main-container">

        <div id="header" class="clearfix">

        <a href="/" id="logo"><img border="0" id="logo_img" alt="Dropbox - Secure backup, sync and sharing made easy." src="/static/16890/images/logo.png"/>
        </a>
    </div>


        <div name="main tabs" id="tabs-container" class="wit_group clearfix">
                <a class='no-tab-link' href="/"><img src="/static/images/icons/icon_spacer.gif" alt="" class="sprite s_arrow_turn_left link-img" />Back to home</a>
        </div>

    <div id="content" style='background:#fff;'  class="clearfix">
        
        <div id="full-content">
                <br class="clear"/>
    <div id="login-page">
        <div id="login-container">
            <div id="subtabs-container" class="clearfix">
                <ul id="subtabs">
                    <li id="login-tab" class="subtab selected"><a href="/login" onclick="Tabs.showTab(this,'login');"><img src="/static/images/icons/icon_spacer.gif" alt="" class="sprite s_key" /> Log in</a></li>
                    <li id="register-tab" class="subtab "><a href="/register" onclick="Tabs.showTab(this,'register');fill_email();"><img src="/static/images/icons/icon_spacer.gif" alt="" class="sprite s_pencil" /> Create an Account</a></li>
                </ul>
            </div>
            <div id="form-container" class="clearfix">

<div id="login-content" class="content-tab" style="">
	<h3 class="heading_text">Log in to Dropbox</h3>
	<form action="/login" id="form" method="post">
<input type="hidden" name="t" value="791206fc33" />
		<table id="login-table">
			<tr>
				<td class="clean"></td>
				<td class="clean"><form:error name="login_email"/></td>
			</tr>
			<tr>
				<td style="width: 55px;" class="label force-no-break">Email</td>
				<td style="width: 210px;">
					<input type="text" id="email" class="textinput" name="login_email" tabindex="3" value=""/>
				</td>
			</tr>
			<tr>
				<td class="clean"></td>
				<td class="clean"><form:error name="login_password"/></td>
			</tr>
			<tr>
				<td class="label force-no-break">Password</td>
				<td>
					<input class="textinput" type="password" name="login_password" tabindex="4"/>
				</td>
			</tr>
			<tr>
				<td class="littlelink">
				</td>
				<td style="padding: 2px 0 0 0;" class="littlelink" align="right">
					<input type="checkbox" class="no-border" name="remember_me" id="remember_me" style="vertical-align: middle;margin: 0;padding: 0; "  tabindex="6" />
					<label for="remember_me" style="vertical-align: middle; cursor: pointer;">Remember me</label>
				</td>
			</tr>
			<tr>
			    <td></td>
			    <td align="right">
			        <a style="text-decoration: none;" href="/forgot" tabindex="-1">Forgot password?</a>
			    </td>
			</tr>
			<tr>
				<td colspan="2" align="right" style="padding-top: 10px;">
					<input type="submit" id="login_submit" name="login_submit" class="button" value="Log in" tabindex="9" />
				</td>
			</tr>
		</table>
	</form>
</div>

<div id="register-content" class="content-tab" style="display:none;">
	<h3 class="heading_text">Create a Dropbox Account</h3>
	<form action="/register" id="form" method="post">
<input type="hidden" name="t" value="5c73879e51" />
		<table>
<tr>
    <td class="clean"></td>
    <td class="clean"><form:error name="fname"/></td>
</tr>



<tr>
    <td style="width: 65px;">
        <div id="fname_l" class="force-no-break label">
            First name
        </div>
    </td>
    <td style="width: 172px">
        <div id="fname_r">
            <input type="text" class="textinput" name="fname" id="fname" tabindex="11"/>
        </div>
    </td>
</tr>
<tr>
    <td class="clean"></td>
    <td class="clean"><form:error name="lname"/></td>
</tr>

<tr>
    <td>
        <div id="lname_l" class="force-no-break label">
            Last name
        </div>
    </td>
    <td>
        <div id="lname_r">
            <input type="text" class="textinput" id="lname" name="lname" tabindex="12"/>
        </div>
    </td>
</tr>
<tr>
    <td colspan="2" class="spacer">
        &nbsp;
    </td>
</tr>
<tr>
    <td class="clean"></td>
    <td class="clean"><form:error name="email"/></td>
</tr>


<tr>
   <td class="label force-no-break">Email</td> 
   <td>
        <input id="email" type="text"  class="textinput" name="email" tabindex="13" value="" />
    </td>
</tr>
<tr>
    <td class="clean"></td>
    <td class="clean"><form:error name="password"/></td>
</tr>
<tr>
    <td class="label force-no-break">
        <div id="choose_pwd">
            Password
        </div>
    </td> 
    <td>
        <input class="textinput" type="password" name="password" tabindex="14"/>
    </td>
</tr>
<tr>
    <td colspan="2" style="text-align: right; padding-right: 1px;">
        <input type="checkbox" class="no-border" checked="checked" name="remember_me" id="remember_me_too" style="vertical-align: middle;"  tabindex="6"/>
        <label for="remember_me" style="vertical-align: middle; cursor: pointer;">Remember me</label>
    </td>
</tr>
		    <tr>
				<td colspan="2" align="right" style="padding-top: 10px;">
					<input type="submit" id="register_submit" name="register_submit" onclick="Forms.disable(this)" class="button" value="Create account" tabindex="19" />
				</td>
			</tr>

		</table>
        <br/>
        <p class="center">
            New to Dropbox? Check out our <a id="tour_link">tour</a>.
            <script type="text/javascript">
                Util.jag('tour_link', 'style', 'text-decoration: none;');
                Util.jag('tour_link', 'href', '/tour');
                Util.jag('tour_link', 'onclick', "window.open(this.href, '_blank'); return false;");                  
            </script>
        </p>
	</form>
</div>
            </div>
        </div>
    </div>

        </div>
        <br class="clear"/>
    </div>


    <div id="ghost-icons" style="cursor: move; height: 0; width:0;"></div>
    <br class="clear"/>


    <!-- Footer -->
    <div name="footer" class="wit_group" id="footer">
        <div style="margin: 0 auto">
            <p id="copyright">
                 <img src="/static/images/icons/icon_spacer.gif" alt="" class="sprite s_gray_logo text-img" />&copy; 2011 Dropbox<br/>
            </p>
            <div id="locale_selector_cont">
                <div id="locale_selector"></div>
            </div>
            <div class="footer-col">
                <ul>
                    <li class="header">Dropbox</li>
                    <li><a href="/">Home</a></li>
                    <li><a href="/install">Install</a></li>
                    <li><a href="/anywhere">Mobile</a></li>
                    <li><a href="/pricing">Pricing</a></li>
                    <li><a href="/features">Features</a></li>
                    <li><a href="/tour">Tour</a></li>
                </ul>
            </div>
            <div class="footer-col">
                <ul>
                    <li class="header">Community</li>
                    <li><a href="/referrals" onmouseup="javascript: MCLog.log('referrals_via_footer');">Referrals</a></li>
                    <li><a href="http://twitter.com/dropbox" target="_blank">Twitter</a></li>
                    <li><a href="http://facebook.com/Dropbox" target="_blank">Facebook</a></li>
                    <li><a href="http://wiki.dropbox.com/">Wiki</a></li>
                    <li><a href="/developers">Developers</a></li>
                    <li><a href="/anywhere/apps">Partners</a></li>

            </ul>
            </div>
            <div class="footer-col">
                <ul>
                    <li class="header">About Us</li>
                    <li><a href="http://blog.dropbox.com/">Dropbox Blog</a></li>
                    <li><a href="/about">Our Team</a></li>
                    <li><a href="/press">Press</a></li>
                    <li><a href="/terms">Policies</a></li>
                    <li><a href="/dmca">Copyright</a></li>
                    <li><a href="http://www.dropbox.com/jobs">Jobs</a></li>
                </ul>
            </div>
            <div class="footer-col">
                <ul>
                    <li class="header">Support</li>
                    <li><a href="/help">Help Center</a></li>
                    <li><a href="http://forums.dropbox.com/">Forums</a></li>
                    <li><a href="/votebox">Votebox</a></li>
                    <li><a href="#" onclick="return Home.showFeedback();">Feedback</a></li>
                    <li><a href="/contact">Contact Us</a></li>
                </ul>
            </div>
           <div class="clear"></div>
        </div>
    </div>
</div>
<script type="text/javascript">
    document.observe("dom:loaded", function() {
        e = $("feedback-link");
        if (e) {
            new Effect.Highlight(e , {duration: 0.5});
        }

        Tabs.init();
        Tabs.check_interval = setInterval("Tabs.check_url('');", 300);
        Dropdown.init();
        Util.focus('email');
        TranslationSuggest.attach_autocomplete();
    });
</script>

<script type="text/javascript">
if (_gat) {
    GA.pageTracker = _gat._getTracker("UA-279179-2");
    Element.observe(window, 'load', function() {
      GA.pageTracker._trackPageview()
      GA.setPendingVars();
    });
}
</script>
<!--[if IE]>
<iframe id="hashkeeper" name='hashkeeper' width="1" height="1" style="display: none" src="/blank" onload="HashKeeper.reloading=false;"></iframe>
<![endif]-->

<div id="ieconsole" style="position: absolute; top: 0; left: 0; font-family: Courier"></div>
<div id="FB_HiddenContainer"  style="position:absolute; top:-10000px;width:0px; height:0px; left: 0;"></div>
<script>
TranslationSuggest.update_i18n_messages({});
</script>



<div id="notice-container" class="clearfix" style='display:none;'>
</div>

</body>
</html>
