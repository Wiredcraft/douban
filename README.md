Douban module is an Drupal integration to douban.com's API

# Workflow

1. A login button will show on login form 
2. User will be redirected to douban.com and is able to login by douban account
3. Generate a temporary password for user on Drupal site
4. Redirect to user on user edit page with welcome message

# Install

1. Download the archive or clone the repository.
2. Copy the douban/ folder in your modules folders (for example: sites/all/modules)
3. Go to the modules pages (/admin/build/modules), and enable the douban module.
4. Apply your douban API key on http://www.douban.com/service/apikey/apply
5. Fill in your API Key and SECRET Key on douban settings page (/admin/settings/douban)
7. From now on, anonymous user is able to see douban button on login form

# Maintainer

- Howell, from Wiredcraft(http://wiredcraft.com)