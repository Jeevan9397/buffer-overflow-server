// controllers.js
angular.module('bfApp')
.controller('LoginCtrl', function($scope, $http, $location) {
  $scope.user = { user: '', pass: '', showPass: false };
  $scope.error = '';

  $scope.togglePass = function() {
    $scope.user.showPass = !$scope.user.showPass;
  };

  $scope.login = function() {
    $http.post('/api/login', {
      user: $scope.user.user,
      pass: $scope.user.pass
    }).then(function(response) {
      if (response.data.status === 'ok') {
        $location.path('/dashboard');
      } else {
        $scope.error = "Login failed. Please try again.";
      }
    }, function() {
      $scope.error = "Invalid username or password.";
    });
  };
})

.controller('DashboardCtrl', function($scope, $http, $location) {
  $scope.records = [];
  $scope.newRecord = {};
  $scope.showAddModal = false;
  $scope.editing = false;
  $scope.selectedRecord = null;

  // Format and Parse Helpers
  function formatDate(isoDate) {
    const d = new Date(isoDate);
    if (!isNaN(d)) {
      const dd = String(d.getDate()).padStart(2, '0');
      const mm = String(d.getMonth() + 1).padStart(2, '0');
      const yyyy = d.getFullYear();
      return `${dd}-${mm}-${yyyy}`;
    }
    return isoDate;
  }

  function parseDate(dobStr) {
  if (dobStr instanceof Date && !isNaN(dobStr)) {
    const yyyy = dobStr.getFullYear();
    const mm = String(dobStr.getMonth() + 1).padStart(2, '0');
    const dd = String(dobStr.getDate()).padStart(2, '0');
    return `${yyyy}-${mm}-${dd}`;
  }

  // Existing fallback for dd-mm-yyyy string
  const parts = dobStr.split('-');
  if (parts.length === 3) {
    return `${parts[2]}-${parts[1]}-${parts[0]}`;
  }

  return dobStr;
}


 function formatCustomers(customers) {
  return customers.map(c => ({
    id: c.id,
    name: c.name,
    father: c.father,
    nationality: c.nationality,
    dob: c.dob,  // raw ISO date for editing
    dobFormatted: formatDate(c.dob),  // only for table display
    birthplace: [c.birth_village, c.birth_district, c.birth_province].filter(Boolean).join(', '),
    address: [c.street, c.city, c.home_district, c.home_province].filter(Boolean).join(', '),
    mobile: c.mobile,
    email: c.email
  }));
}


  function refreshRecords() {
    $http.get('/api/customers').then(function(resp) {
      $scope.records = formatCustomers(resp.data.customers);
    });
  }

  refreshRecords();

  $scope.add = function() {
    $scope.newRecord = {};
    $scope.editing = false;
    $scope.showAddModal = true;
  };

  $scope.cancelAdd = function() {
    $scope.showAddModal = false;
    $scope.editing = false;
    $scope.newRecord = {};
  };

  $scope.selectRecord = function(r) {
    $scope.selectedRecord = r;
  };

  $scope.triggerEdit = function() {
    if (!$scope.selectedRecord) {
      alert("Please select a record to edit.");
      return;
    }
    $scope.editRecord($scope.selectedRecord);
  };

  $scope.editRecord = function(record) {
    $scope.newRecord = angular.copy(record);
    $scope.newRecord.dob = new Date(record.dob);
    $scope.editing = true;
    $scope.showAddModal = true;
  };

  $scope.saveRecord = function() {
    const parts = ($scope.newRecord.birthplace || '').split(',').map(s => s.trim());
    const addrParts = ($scope.newRecord.address || '').split(',').map(s => s.trim());

    const payload = {
      name: $scope.newRecord.name,
      father: $scope.newRecord.father,
      nationality: $scope.newRecord.nationality,
      dob: $scope.newRecord.dob,
      birth_village: parts[0] || '',
      birth_district: parts[1] || '',
      birth_province: parts[2] || '',
      street: addrParts[0] || '',
      city: addrParts[1] || '',
      home_district: addrParts[2] || '',
      home_province: addrParts[3] || '',
      mobile: $scope.newRecord.mobile,
      email: $scope.newRecord.email
    };

    const endpoint = $scope.editing
      ? `/api/customers/${$scope.newRecord.id}`
      : '/api/customers';

    const method = $scope.editing ? $http.put : $http.post;

    method(endpoint, payload).then(function() {
      refreshRecords();
      $scope.showAddModal = false;
      $scope.editing = false;
      $scope.newRecord = {};
    }, function() {
      alert("Error saving record.");
    });
  };

  $scope.logout = function() {
    $location.path('/login');
  };
});
